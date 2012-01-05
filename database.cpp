#include "database.h"

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/lexical_cast.hpp>
#include <gdcmAnonymizer.h>
#include <gdcmAttribute.h>
#include <gdcmByteValue.h>
#include <gdcmDataSet.h>
#include <gdcmFile.h>
#include <gdcmFileMetaInformation.h>
#include <gdcmUIDGenerator.h>
#include <gdcmSystem.h>
#include <gdcmWriter.h>
#include <magic.h>

#include "dicom_to_cpp.h"

class BSONBuilderAction
{
public :
    BSONBuilderAction(mongo::BSONObjBuilder * builder)
    : _builder(builder), _array_builder(NULL)
    {
    }

    mongo::BSONObjBuilder * get_builder() const
    {
        return this->_builder;
    }

    void begin_data_set(gdcm::DataSet const & data_set)
    {
    }

    void data_element(gdcm::DataElement const & data_element)
    {
        gdcm::Tag const & tag = data_element.GetTag();

        if(tag == gdcm::Tag(0x7fe0,0x0010) || tag == gdcm::Tag(0x0042,0x0011))
        {
            // Pixel Data, Encapsulated Document
            return;
        }
        if(tag.GetGroup()/256==0x60 && tag.GetElement()==0x3000)
        {
            // Overlay Data (60xx,3000)
            return;
        }
        parse(data_element, *this);
    }

    void sequence(gdcm::DataElement const & sequence)
    {
        mongo::BSONArrayBuilder nested_array_builder;
        for(gdcm::SequenceOfItems::ConstIterator it=sequence.GetValueAsSQ()->Begin();
            it!=sequence.GetValueAsSQ()->End(); ++it)
        {
            mongo::BSONObjBuilder nested_builder;
            BSONBuilderAction nested_action(&nested_builder);
            parse(it->GetNestedDataSet(), nested_action);
            nested_array_builder.append(nested_builder.obj());
        }

        this->_builder->append(sequence.GetTag().PrintAsPipeSeparatedString(),
                               nested_array_builder.arr());
    }

    void end_data_set(gdcm::DataSet const & data_set)
    {
    }

    template<gdcm::VR::VRType VVR>
    void single_valued_element(gdcm::DataElement const & data_element)
    {
        this->_builder->append(data_element.GetTag().PrintAsPipeSeparatedString(),
                              parse<VVR>(data_element));
    }

    template<gdcm::VR::VRType VVR>
    void begin_multiple_valued_element(gdcm::DataElement const & data_element)
    {
        if(this->_array_builder != NULL)
        {
            delete this->_array_builder;
        }
        this->_array_builder = new mongo::BSONArrayBuilder();
    }

    template<gdcm::VR::VRType VVR>
    void multiple_valued_element(gdcm::DataElement const & data_element, unsigned long index)
    {
        this->_array_builder->append(parse<VVR>(data_element, index));
    }

    template<gdcm::VR::VRType VVR>
    void end_multiple_valued_element(gdcm::DataElement const & data_element)
    {
        this->_builder->append(data_element.GetTag().PrintAsPipeSeparatedString(),
                               this->_array_builder->arr());
    }

private :
    mongo::BSONObjBuilder * _builder;
    mongo::BSONArrayBuilder * _array_builder;
};

Database
::Database(std::string const & db_name, std::string const & host, unsigned int port)
: _db_name(db_name)
{
    this->_connection.connect(host+":"+boost::lexical_cast<std::string>(port));
    this->_grid_fs = new mongo::GridFS(this->_connection, this->_db_name);
}

Database
::~Database()
{
    delete this->_grid_fs;
}
    
void 
Database
::insert_user(mongo::BSONObj const & user)
{
    if(!this->_connection.findOne(
           this->_db_name+".users", 
           QUERY("id" << user.getStringField("id"))).isEmpty())
    {
        std::ostringstream message;
        message << "Cannot insert user \"" << user.getStringField("id") << "\""
                << " : already exists";
        throw std::runtime_error(message.str());
    }
    
    this->_connection.insert(this->_db_name+".users", user);
}

void 
Database
::insert_protocol(mongo::BSONObj const & protocol)
{
    if(!this->_connection.findOne(this->_db_name+".protocols", QUERY("id" << protocol.getStringField("id"))).isEmpty())
    {
        std::ostringstream message;
        message << "Cannot insert protocol \"" << protocol.getStringField("id") << "\""
                << " : already exists";
        throw std::runtime_error(message.str());
    }
    
    if(this->_connection.findOne(this->_db_name+".users", QUERY("id" << protocol.getStringField("sponsor"))).isEmpty())
    {
        std::ostringstream message;
        message << "Cannot insert protocol \"" << protocol.getStringField("id") << "\""
                << " : no such sponsor \"" << protocol.getStringField("sponsor") << "\"";
        throw std::runtime_error(message.str());
    }
    
    this->_connection.insert(this->_db_name+".protocols", protocol);
}

void 
Database
::insert_file(std::string const & filename)
{
    gdcm::DataSet dataset;

    // SOP Class UID
    {
        gdcm::Attribute<0x0008,0x0016> attribute;
        attribute.SetValue("1.2.840.10008.5.1.4.1.1.66"); // Raw Data Storage
        dataset.Insert(attribute.GetAsDataElement());
    }

    // SOP Instance UID
    {
        gdcm::Attribute<0x0008,0x0018> attribute;
        attribute.SetValue(std::string(gdcm::UIDGenerator().Generate()));
        dataset.Insert(attribute.GetAsDataElement());
    }

    // Instance Number
    {
        gdcm::Attribute<0x0020,0x0013> attribute;
        dataset.Insert(attribute.GetAsDataElement());
    }

    // MIME Type of Encapsulated Document
    {
        magic_t cookie = magic_open(MAGIC_MIME_TYPE|MAGIC_MIME_ENCODING);
        magic_load(cookie, NULL);
        char const * const type = magic_file(cookie, filename.c_str());

        gdcm::Attribute<0x0042,0x0012> attribute;
        attribute.SetValue(type);
        dataset.Insert(attribute.GetAsDataElement());

        magic_close(cookie);
    }

    // Encapsulated Document
    {
        std::ifstream stream(filename.c_str());
        // Find file length and allocate buffer
        stream.seekg(0, std::ios_base::end);
        long int const length = stream.tellg();
        std::vector<char> buffer(length);
        // Go back to beginning and read the file
        stream.seekg(0, std::ios_base::beg);
        stream.read(&buffer[0], buffer.size());

        gdcm::DataElement data_element(gdcm::Tag(0x0042,0x0011));
        data_element.SetVR(gdcm::VR::OB);
        data_element.SetByteValue(&buffer[0], buffer.size());
        dataset.Insert(data_element);
    }

    std::cout << dataset << std::endl;

    //this->insert_dataset(dataset);
}

void
Database::
insert_dataset(gdcm::DataSet const & dataset)
{
    mongo::BSONObjBuilder builder;
    BSONBuilderAction action(&builder);
    parse(dataset, action);
    this->_connection.insert(this->_db_name+".documents", builder.obj());

    gdcm::FileMetaInformation header;
    header.FillFromDataSet(dataset);
    header.SetDataSetTransferSyntax(gdcm::TransferSyntax::ExplicitVRLittleEndian);

    // This code crashes when a gdcm::File (without the SmartPointer) is used
    gdcm::SmartPointer<gdcm::File> file = new gdcm::File();
    file->SetHeader(header);
    file->SetDataSet(dataset);

    char filename[] = "/tmp/XXXXXX";
    int const fd = mkstemp(filename);
    close(fd);

    gdcm::Writer writer;
    writer.SetFile(*file);
    writer.SetFileName(filename);
    writer.Write();

    gdcm::Attribute<0x0008,0x0018> attribute;
    attribute.SetFromDataElement(dataset.GetDataElement(gdcm::Tag(0x0008,0x0018)));
    this->_grid_fs->storeFile(filename, attribute.GetValue());

    gdcm::System::RemoveFile(filename);
}

mongo::auto_ptr<mongo::DBClientCursor> 
Database
::query(mongo::Query const & query, mongo::BSONObj* fields, std::string const & ns)
{
    return this->_connection.query(this->_db_name+"."+ns, query, 0, 0, fields);
}

gdcm::DataSet
Database::de_identify(gdcm::DataSet const & dataset) const
{
    // PS 3.15 - 2008
    // Table E.1-1
    // BALCPA
    gdcm::Tag BasicApplicationLevelConfidentialityProfileAttributes[] = {
//              Attribute Name                                            Tag
/*              Instance Creator UID                                  */ gdcm::Tag(0x0008,0x0014),
/*              SOP Instance UID                                      */ gdcm::Tag(0x0008,0x0018),
/*              Accession Number                                      */ gdcm::Tag(0x0008,0x0050),
/*              Institution Name                                      */ gdcm::Tag(0x0008,0x0080),
/*              Institution Address                                   */ gdcm::Tag(0x0008,0x0081),
/*              Referring Physician's Name                            */ gdcm::Tag(0x0008,0x0090),
/*              Referring Physician's Address                         */ gdcm::Tag(0x0008,0x0092),
/*              Referring Physician's Telephone Numbers               */ gdcm::Tag(0x0008,0x0094),
/*              Station Name                                          */ gdcm::Tag(0x0008,0x1010),
/*              Study Description                                     */ gdcm::Tag(0x0008,0x1030),
/*              Series Description                                    */ gdcm::Tag(0x0008,0x103E),
/*              Institutional Department Name                         */ gdcm::Tag(0x0008,0x1040),
/*              Physician(s) of Record                                */ gdcm::Tag(0x0008,0x1048),
/*              Performing Physicians' Name                           */ gdcm::Tag(0x0008,0x1050),
/*              Name of Physician(s) Reading Study                    */ gdcm::Tag(0x0008,0x1060),
/*              Operators' Name                                       */ gdcm::Tag(0x0008,0x1070),
/*              Admitting Diagnoses Description                       */ gdcm::Tag(0x0008,0x1080),
/*              Referenced SOP Instance UID                           */ gdcm::Tag(0x0008,0x1155),
/*              Derivation Description                                */ gdcm::Tag(0x0008,0x2111),
/*              Patient's Name                                        */ gdcm::Tag(0x0010,0x0010),
/*              Patient ID                                            */ gdcm::Tag(0x0010,0x0020),
/*              Patient's Birth Date                                  */ gdcm::Tag(0x0010,0x0030),
/*              Patient's Birth Time                                  */ gdcm::Tag(0x0010,0x0032),
/*              Patient's Sex                                         */ gdcm::Tag(0x0010,0x0040),
/*              Other Patient Ids                                     */ gdcm::Tag(0x0010,0x1000),
/*              Other Patient Names                                   */ gdcm::Tag(0x0010,0x1001),
/*              Patient's Age                                         */ gdcm::Tag(0x0010,0x1010),
/*              Patient's Size                                        */ gdcm::Tag(0x0010,0x1020),
/*              Patient's Weight                                      */ gdcm::Tag(0x0010,0x1030),
/*              Medical Record Locator                                */ gdcm::Tag(0x0010,0x1090),
/*              Ethnic Group                                          */ gdcm::Tag(0x0010,0x2160),
/*              Occupation                                            */ gdcm::Tag(0x0010,0x2180),
/*              Additional Patient's History                          */ gdcm::Tag(0x0010,0x21B0),
/*              Patient Comments                                      */ gdcm::Tag(0x0010,0x4000),
/*              Device Serial Number                                  */ gdcm::Tag(0x0018,0x1000),
/*              Protocol Name                                         */ gdcm::Tag(0x0018,0x1030),
/*              Study Instance UID                                    */ gdcm::Tag(0x0020,0x000D),
/*              Series Instance UID                                   */ gdcm::Tag(0x0020,0x000E),
/*              Study ID                                              */ gdcm::Tag(0x0020,0x0010),
/*              Frame of Reference UID                                */ gdcm::Tag(0x0020,0x0052),
/*              Synchronization Frame of Reference UID                */ gdcm::Tag(0x0020,0x0200),
/*              Image Comments                                        */ gdcm::Tag(0x0020,0x4000),
/*              Request Attributes Sequence                           */ gdcm::Tag(0x0040,0x0275),
/*              UID                                                   */ gdcm::Tag(0x0040,0xA124),
/*              Content Sequence                                      */ gdcm::Tag(0x0040,0xA730),
/*              Storage Media File-set UID                            */ gdcm::Tag(0x0088,0x0140),
/*              Referenced Frame of Reference UID                     */ gdcm::Tag(0x3006,0x0024),
/*              Related Frame of Reference UID                        */ gdcm::Tag(0x3006,0x00C2)
};

    unsigned int const deidSize = sizeof(gdcm::Tag);
    unsigned int const numDeIds = sizeof(BasicApplicationLevelConfidentialityProfileAttributes) / deidSize;
    gdcm::Tag const *start = BasicApplicationLevelConfidentialityProfileAttributes;
    gdcm::Tag const *end = start + numDeIds;

    gdcm::Anonymizer anonymizer;
    anonymizer.GetFile().SetDataSet(dataset);
    
    for(gdcm::Tag const * it=start; it!=end; ++it)
    {
        anonymizer.Empty(*it);
    }

    // Generated a new SOP Instance UID for this dataset
    std::string const sop_instance_uid = gdcm::UIDGenerator().Generate();
    gdcm::Attribute<0x0008,0x0018> attribute;
    attribute.SetValue(sop_instance_uid);

    gdcm::DataSet de_identified = anonymizer.GetFile().GetDataSet();
    de_identified.Replace(attribute.GetAsDataElement());

    return de_identified;
}

void 
Database
::set_clinical_trial_informations(gdcm::DataSet & dataset, std::string const & sponsor, std::string const & protocol, std::string const &subject)
{
    if(this->_connection.findOne(this->_db_name+".users", QUERY("id" << sponsor)).isEmpty())
    {
        std::ostringstream message;
        message << "Cannot set Clinical Trial informations : no such sponsor \"" << sponsor << "\"";
        throw std::runtime_error(message.str());
    }
    
    if(this->_connection.findOne(this->_db_name+".protocols", QUERY("id" << protocol)).isEmpty())
    {
        std::ostringstream message;
        message << "Cannot set Clinical Trial informations : no such protocol \"" << protocol << "\"";
        throw std::runtime_error(message.str());
    }
    
    {
        gdcm::DataElement de(gdcm::Tag(0x0012,0x0010));
        de.SetVR(gdcm::VR::LO);
        gdcm::LOComp value(sponsor);
        de.SetByteValue(value, value.length());
        dataset.Insert(de);
    }
    {
        gdcm::DataElement de(gdcm::Tag(0x0012,0x0020));
        de.SetVR(gdcm::VR::LO);
        gdcm::LOComp value(protocol);
        de.SetByteValue(value, value.length());
        dataset.Insert(de);
    }
    {
        gdcm::DataElement de(gdcm::Tag(0x0012,0x0040));
        de.SetVR(gdcm::VR::LO);
        gdcm::LOComp value(subject);
        de.SetByteValue(value, value.length());
        dataset.Insert(de);
    }
}

void
Database
::get_file(std::string const sop_instance_uid, std::ostream & stream) const
{
    mongo::GridFile file = this->_grid_fs->findFile(sop_instance_uid);
    if(!file.exists())
    {
        std::ostringstream message;
        message << "No file with SOP Instance UID \""
                << sop_instance_uid << "\"";
        throw std::runtime_error(message.str());
    }
    file.write(stream);
}
