#include "database.h"

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
#include "user.h"

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
::insert_user(User const & user)
{
    if(this->_connection.count(this->_db_name+".users", BSON("id" << user.get_id()))>0)
    {
        std::ostringstream message;
        message << "Cannot insert user \"" << user.get_id()<< "\""
                << " : already exists";
        throw std::runtime_error(message.str());
    }
    
    this->_connection.insert(this->_db_name+".users", user.to_bson());
}

void 
Database
::insert_protocol(mongo::BSONObj const & protocol)
{
    if(this->_connection.count(
            this->_db_name+".protocols",
            BSON("id" << protocol.getStringField("id")))>0)
    {
        std::ostringstream message;
        message << "Cannot insert protocol \"" << protocol.getStringField("id") << "\""
                << " : already exists";
        throw std::runtime_error(message.str());
    }
    
    if(this->_connection.count(
            this->_db_name+".users",
            BSON("id" << protocol.getStringField("sponsor")))==0)
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
::insert_file(std::string const & filename, User const & sponsor, std::string const & protocol, std::string const & subject)
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

    magic_t cookie = magic_open(MAGIC_MIME_TYPE|MAGIC_MIME_ENCODING);
    magic_load(cookie, NULL);
    std::string const mime_type(magic_file(cookie, filename.c_str()));
    magic_close(cookie);
    // MIME Type of Encapsulated Document
    {
        gdcm::Attribute<0x0042,0x0012> attribute;
        attribute.SetValue(mime_type);
        dataset.Insert(attribute.GetAsDataElement());
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

    this->set_clinical_trial_informations(dataset, sponsor, protocol, subject);

    // TODO : refactor the code with insert_dataset. Only difference is 
    // value of original_mime_type
    mongo::BSONObjBuilder builder;
    BSONBuilderAction action(&builder);
    parse(dataset, action);
    builder << "original_mime_type" << mime_type;
    this->_connection.insert(this->_db_name+".documents", builder.obj());

    char temp_filename[] = "/tmp/XXXXXX";
    int const fd = mkstemp(temp_filename);
    close(fd);

    gdcm::Writer writer;
    writer.GetFile().SetDataSet(dataset);
    writer.SetFileName(temp_filename);
    writer.Write();

    gdcm::Attribute<0x0008,0x0018> attribute;
    attribute.Set(dataset);
    this->_grid_fs->storeFile(filename, attribute.GetValue());

    gdcm::System::RemoveFile(temp_filename);
}

void
Database
::insert_dataset(gdcm::DataSet const & dataset)
{
    mongo::BSONObjBuilder builder;
    BSONBuilderAction action(&builder);
    parse(dataset, action);
    builder << "original_mime_type" << "application/dicom; charset=binary";
    this->_connection.insert(this->_db_name+".documents", builder.obj());

    char filename[] = "/tmp/XXXXXX";
    int const fd = mkstemp(filename);
    close(fd);

    gdcm::Writer writer;
    writer.GetFile().SetDataSet(dataset);
    writer.SetFileName(filename);
    writer.Write();

    gdcm::Attribute<0x0008,0x0018> attribute;
    attribute.Set(dataset);
    this->_grid_fs->storeFile(filename, attribute.GetValue());

    gdcm::System::RemoveFile(filename);
}

std::vector<User>
Database
::query_users(mongo::Query const & query)
{
    std::vector<std::string> fields;
    
    mongo::auto_ptr<mongo::DBClientCursor> cursor = 
        this->_query("users", query, fields);
    
    std::vector<User> result;
    while(cursor->more())
    {
        mongo::BSONObj const & object = cursor->next();
        User user;
        user.from_bson(object);
        result.push_back(user);
    }
    
    return result;
}

mongo::auto_ptr<mongo::DBClientCursor>
Database
::query_protocols(mongo::Query const & query)
{
    std::vector<std::string> fields;
    return this->query_protocols(query, fields);
}

mongo::auto_ptr<mongo::DBClientCursor>
Database
::query_protocols(mongo::Query const & query, std::vector<std::string> const & fields)
{
    return this->_query("protocols", query, fields);
}

mongo::auto_ptr<mongo::DBClientCursor>
Database
::query_documents(mongo::Query const & query)
{
    std::vector<std::string> fields;
    fields.push_back("(0008|0018)");
    return this->query_documents(query, fields);
}

mongo::auto_ptr<mongo::DBClientCursor>
Database
::query_documents(mongo::Query const & query, std::vector<std::string> const & fields)
{
    return this->_query("documents", query, fields);
}

gdcm::DataSet
Database::de_identify(gdcm::DataSet const & dataset) const
{
    // PS 3.15 - 2008
    // Table E.1-1
    // BALCPA
    gdcm::Tag to_remove[] = {
        gdcm::Tag(0x0008,0x0014), //Instance Creator UID
        gdcm::Tag(0x0008,0x0018), //SOP Instance UID
        gdcm::Tag(0x0008,0x0018), //Accession Number
        gdcm::Tag(0x0008,0x0080), //Institution Name
        gdcm::Tag(0x0008,0x0081), //Institution Address
        gdcm::Tag(0x0008,0x0090), //Referring Physician's Name
        gdcm::Tag(0x0008,0x0092), //Referring Physician's Address
        gdcm::Tag(0x0008,0x0094), //Referring Physician's Telephone Numbers
        gdcm::Tag(0x0008,0x1010), //Station Name
        gdcm::Tag(0x0008,0x1030), //Study Description
        gdcm::Tag(0x0008,0x103E), //Series Description
        gdcm::Tag(0x0008,0x1040), //Institutional Department Name
        gdcm::Tag(0x0008,0x1048), //Physician(s) of Record
        gdcm::Tag(0x0008,0x1050), //Performing Physicians' Name
        gdcm::Tag(0x0008,0x1060), //Name of Physician(s) Reading Study
        gdcm::Tag(0x0008,0x1070), //Operators' Name
        gdcm::Tag(0x0008,0x1080), //Admitting Diagnoses Description
        gdcm::Tag(0x0008,0x1155), //Referenced SOP Instance UID
        gdcm::Tag(0x0008,0x2111), //Derivation Description
        gdcm::Tag(0x0010,0x0010), //Patient's Name
        gdcm::Tag(0x0010,0x0020), //Patient ID
        gdcm::Tag(0x0010,0x0030), //Patient's Birth Date
        gdcm::Tag(0x0010,0x0032), //Patient's Birth Time
        gdcm::Tag(0x0010,0x0040), //Patient's Sex
        gdcm::Tag(0x0010,0x1000), //Other Patient Ids
        gdcm::Tag(0x0010,0x1001), //Other Patient Names
        gdcm::Tag(0x0010,0x1010), //Patient's Age
        gdcm::Tag(0x0010,0x1020), //Patient's Size
        gdcm::Tag(0x0010,0x1030), //Patient's Weight
        gdcm::Tag(0x0010,0x1090), //Medical Record Locator
        gdcm::Tag(0x0010,0x2160), //Ethnic Group
        gdcm::Tag(0x0010,0x2180), //Occupation
        gdcm::Tag(0x0010,0x21B0), //Additional Patient's History
        gdcm::Tag(0x0010,0x4000), //Patient Comments
        gdcm::Tag(0x0018,0x1000), //Device Serial Number
        gdcm::Tag(0x0018,0x1030), //Protocol Name
        gdcm::Tag(0x0020,0x000D), //Study Instance UID
        gdcm::Tag(0x0020,0x000E), //Series Instance UID
        gdcm::Tag(0x0020,0x0010), //Study ID
        gdcm::Tag(0x0020,0x0052), //Frame of Reference UID
        gdcm::Tag(0x0020,0x0200), //Synchronization Frame of Reference UID
        gdcm::Tag(0x0020,0x4000), //Image Comments
        gdcm::Tag(0x0040,0x0275), //Request Attributes Sequence
        gdcm::Tag(0x0040,0xA124), //UID
        gdcm::Tag(0x0040,0xA730), //Content Sequence
        gdcm::Tag(0x0088,0x0140), //Storage Media File-set UID
        gdcm::Tag(0x3006,0x0024), //Referenced Frame of Reference UID
        gdcm::Tag(0x3006,0x00C2), //Related Frame of Reference UID
    };

    gdcm::Tag * to_remove_end = to_remove+sizeof(to_remove)/sizeof(gdcm::Tag);

    gdcm::DataSet de_identified;
    for(gdcm::DataSet::ConstIterator it=dataset.Begin(); it!=dataset.End(); ++it)
    {
        if(std::find(to_remove, to_remove_end, it->GetTag()) == to_remove_end)
        {
            de_identified.Insert(*it);
        }
    }

    // Generated a new SOP Instance UID for the anonymized dataset
    std::string const sop_instance_uid = gdcm::UIDGenerator().Generate();
    gdcm::Attribute<0x0008,0x0018> attribute;
    attribute.SetValue(sop_instance_uid);
    de_identified.Replace(attribute.GetAsDataElement());

    return de_identified;
}

void 
Database
::set_clinical_trial_informations(gdcm::DataSet & dataset, User const & sponsor, std::string const & protocol, std::string const &subject)
{
    if(this->_connection.count(this->_db_name+".users", BSON("id" << sponsor.get_id())) == 0)
    {
        std::ostringstream message;
        message << "Cannot set Clinical Trial informations : no such sponsor \"" << sponsor.get_id() << "\"";
        throw std::runtime_error(message.str());
    }
    
    if(this->_connection.count(this->_db_name+".protocols", BSON("id" << protocol)) == 0)
    {
        std::ostringstream message;
        message << "Cannot set Clinical Trial informations : no such protocol \"" << protocol << "\"";
        throw std::runtime_error(message.str());
    }
    
    gdcm::Attribute<0x0012,0x0010> clinical_trial_sponsor_name;
    clinical_trial_sponsor_name.SetValue(sponsor.get_id());
    dataset.Insert(clinical_trial_sponsor_name.GetAsDataElement());

    gdcm::Attribute<0x0012,0x0020> clinical_trial_protocol_id;
    clinical_trial_protocol_id.SetValue(protocol);
    dataset.Insert(clinical_trial_protocol_id.GetAsDataElement());

    gdcm::Attribute<0x0012,0x0040> clinical_trial_subject_id;
    clinical_trial_subject_id.SetValue(subject);
    dataset.Insert(clinical_trial_subject_id.GetAsDataElement());
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

mongo::auto_ptr<mongo::DBClientCursor>
Database
::_query(std::string const & ns, mongo::Query const & query, std::vector<std::string> const & fields)
{
    if(!fields.empty())
    {
        mongo::BSONObjBuilder builder;
        for(std::vector<std::string>::const_iterator it=fields.begin(); it!=fields.end();
            ++it)
        {
            builder << *it << 1;
        }
        mongo::BSONObj bson_fields(builder.obj());

        return this->_connection.query(this->_db_name+"."+ns, query, 0, 0, &bson_fields);
    }
    else
    {
        return this->_connection.query(this->_db_name+"."+ns, query);
    }
}
