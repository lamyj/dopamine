#include "database.h"

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/lexical_cast.hpp>
#include <gdcmAnonymizer.h>
#include <gdcmAttribute.h>
#include <gdcmDataSet.h>
#include <gdcmFile.h>
#include <gdcmFileMetaInformation.h>

Database
::Database(std::string const & db_name, std::string const & host, unsigned int port)
: _db_name(db_name)
{
    this->_connection.connect(host+":"+boost::lexical_cast<std::string>(port));
    //this->_grid_fs = GridFS(this->_connection, this->_db_name);
    
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
Database::
insert_dataset(gdcm::DataSet const & dataset)
{
    gdcm::UIComp sop_instance_uid;
    {
        gdcm::DataElement const & de = 
            dataset.GetDataElement(gdcm::Tag(0x0008,0x0018));
        gdcm::Attribute<0x0008,0x0018> at;
        at.SetFromDataElement(de);
        sop_instance_uid = at.GetValue().Trim();
    }
    
    if(!this->_connection.findOne(this->_db_name+".documents", QUERY("_original_sop_instance_uid" << sop_instance_uid)).isEmpty())
    {
        throw std::runtime_error("Cannot insert dataset : already in database");
    }
        
    mongo::BSONObj document = BSON("_original_sop_instance_uid" << sop_instance_uid);
    
    this->_dataset_to_document(dataset, document);
    this->_connection.insert(this->_db_name+".documents", document);
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

    return anonymizer.GetFile().GetDataSet();
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

// GDCM forces an even-length string, we don't want this
template<gdcm::VR::VRType VVR>
struct VRToType;

template<> struct VRToType<gdcm::VR::AE> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::AS> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::CS> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::DS> { typedef double Type; };
template<> struct VRToType<gdcm::VR::DA> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::DT> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::FL> { typedef float Type; };
template<> struct VRToType<gdcm::VR::FD> { typedef double Type; };
template<> struct VRToType<gdcm::VR::IS> { typedef int32_t Type; };
template<> struct VRToType<gdcm::VR::LO> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::LT> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::PN> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::SH> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::SL> { typedef int32_t Type; };
template<> struct VRToType<gdcm::VR::SS> { typedef int16_t Type; };
template<> struct VRToType<gdcm::VR::ST> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::TM> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::UI> { typedef std::string Type; };
template<> struct VRToType<gdcm::VR::UL> { typedef uint32_t Type; };
template<> struct VRToType<gdcm::VR::US> { typedef uint16_t Type; };
template<> struct VRToType<gdcm::VR::UT> { typedef std::string Type; };

template<typename T>
T parse_ascii(gdcm::DataElement const & data_element, unsigned long index, bool is_padding_significant)
{
    char const * const data = data_element.GetByteValue()->GetPointer();
    gdcm::VL const length = data_element.GetVL();

    char const * begin = data;
    for(unsigned long i=0; i<index; ++i)
    {
        begin = std::find(begin, data+length, '\\');
        ++begin;
    }
    char const * end = std::find(begin, data+length, '\\');

    if(!is_padding_significant)
    {
        while(*begin == ' ')
        {
            ++begin;
        }

        if(end == data+length)
        {
            --end;
        }
        while(*end == ' ' || *end == '\\' || *end == '\0')
        {
            --end;
        }
        ++end;
    }

    return T(std::string(begin, end));
}

template<typename T>
T parse_binary(gdcm::DataElement const & data_element, unsigned long index=0)
{
    T const * begin = reinterpret_cast<T const *>(data_element.GetByteValue()->GetPointer());
    return *(begin+index);
}

template<gdcm::VR::VRType VVR>
typename VRToType<VVR>::Type parse(gdcm::DataElement const & data_element, unsigned long index=0);

template<>
VRToType<gdcm::VR::AE>::Type
parse<gdcm::VR::AE>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::AE>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::AS>::Type
parse<gdcm::VR::AS>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::AS>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::CS>::Type
parse<gdcm::VR::CS>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::CS>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::DA>::Type
parse<gdcm::VR::DA>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::DA>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::DT>::Type
parse<gdcm::VR::DT>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::DT>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::DS>::Type
parse<gdcm::VR::DS>(gdcm::DataElement const & data_element, unsigned long index)
{
    std::string const value = parse_ascii<std::string>(data_element, index, false);
    std::istringstream in(value);
    in.imbue(std::locale("C"));
    VRToType<gdcm::VR::DS>::Type result;
    in >> result;
    return result;
}

template<>
VRToType<gdcm::VR::FL>::Type
parse<gdcm::VR::FL>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::FL>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::FD>::Type
parse<gdcm::VR::FD>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::FD>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::IS>::Type
parse<gdcm::VR::IS>(gdcm::DataElement const & data_element, unsigned long index)
{
    std::string const value = parse_ascii<std::string>(data_element, index, false);
    std::istringstream in(value);
    in.imbue(std::locale("C"));
    VRToType<gdcm::VR::IS>::Type result;
    in >> result;
    return result;
}

template<>
VRToType<gdcm::VR::LO>::Type
parse<gdcm::VR::LO>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::LO>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::LT>::Type
parse<gdcm::VR::LT>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::LT>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::PN>::Type
parse<gdcm::VR::PN>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::PN>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::SH>::Type
parse<gdcm::VR::SH>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::SH>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::SL>::Type
parse<gdcm::VR::SL>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::SL>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::SS>::Type
parse<gdcm::VR::SS>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::SS>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::ST>::Type
parse<gdcm::VR::ST>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::ST>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::TM>::Type
parse<gdcm::VR::TM>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::TM>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::UI>::Type
parse<gdcm::VR::UI>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::UI>::Type>(data_element, index, false);
}

template<>
VRToType<gdcm::VR::UL>::Type
parse<gdcm::VR::UL>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::UL>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::US>::Type
parse<gdcm::VR::US>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_binary<VRToType<gdcm::VR::US>::Type>(data_element, index);
}

template<>
VRToType<gdcm::VR::UT>::Type
parse<gdcm::VR::UT>(gdcm::DataElement const & data_element, unsigned long index)
{
    return parse_ascii<VRToType<gdcm::VR::UT>::Type>(data_element, index, false);
}

void parse(gdcm::DataElement const & data_element, mongo::BSONObjBuilder & builder)
{
    unsigned long count=0;

    if(data_element.GetVR() & (gdcm::VR::LT | gdcm::VR::ST | gdcm::VR::UT))
    {
        // LT, ST and UT may not be multi-valued
        count=1;
    }
    else if(gdcm::VR::IsASCII(data_element.GetVR()))
    {
        count = 1+std::count(
            data_element.GetByteValue()->GetPointer(),
            data_element.GetByteValue()->GetPointer()+data_element.GetVL(),
            '\\');
    }
    else
    {
        count = data_element.GetVL()/data_element.GetVR().GetSize();
    }

#define VR_CASE(vr) \
    if(vr == data_element.GetVR()) \
    { \
        if(count == 1) \
        { \
            builder.append(data_element.GetTag().PrintAsPipeSeparatedString(), \
                           parse<vr>(data_element)); \
        } \
        else \
        { \
            mongo::BSONArrayBuilder array_builder; \
            for(unsigned long i=0; i<count; ++i) \
            { \
                array_builder.append(parse<vr>(data_element, i)); \
            } \
            builder.append(data_element.GetTag().PrintAsPipeSeparatedString(), \
                           array_builder.arr()); \
        }\
    }

    VR_CASE(gdcm::VR::AE);
    VR_CASE(gdcm::VR::AS);
//    VR_CASE(gdcm::VR::AT);
    VR_CASE(gdcm::VR::CS);
    VR_CASE(gdcm::VR::DA);
    VR_CASE(gdcm::VR::DS);
    VR_CASE(gdcm::VR::DT);
    VR_CASE(gdcm::VR::FD);
    VR_CASE(gdcm::VR::FL);
    VR_CASE(gdcm::VR::IS);
    VR_CASE(gdcm::VR::LO);
    VR_CASE(gdcm::VR::LT);
//    VR_CASE(gdcm::VR::OB);
//    VR_CASE(gdcm::VR::OF);
//    VR_CASE(gdcm::VR::OW);
    VR_CASE(gdcm::VR::PN);
    VR_CASE(gdcm::VR::SH);
    VR_CASE(gdcm::VR::SL);
    // SQ is special case !
//    VR_CASE(gdcm::VR::SQ);
    VR_CASE(gdcm::VR::SS);
    VR_CASE(gdcm::VR::ST);
    VR_CASE(gdcm::VR::TM);
    VR_CASE(gdcm::VR::UI);
    VR_CASE(gdcm::VR::UL);
//    VR_CASE(gdcm::VR::UN);
    VR_CASE(gdcm::VR::US);
    VR_CASE(gdcm::VR::UT);

#undef VR_CASE
}

void
Database
::_dataset_to_document(gdcm::DataSet const & dataset, mongo::BSONObj & document)
{
    mongo::BSONObjBuilder builder;
    
    for(gdcm::DataSet::ConstIterator it=dataset.Begin();
        it!=dataset.End(); ++it)
    {
        if(it->GetTag() == gdcm::Tag(0x7fe0,0x0010) ||
           it->GetTag() == gdcm::Tag(0x0042,0x0011))
        {
            // Pixel Data, Encapsulated Document
            continue;
        }
        if(it->GetTag().GetGroup()/256==0x60 && it->GetTag().GetElement()==0x3000)
        {
            // Overlay Data (60xx,3000)
            continue;
        }
        
        // Get value
        parse(*it, builder);
    }
}
