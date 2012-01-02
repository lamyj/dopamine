#include "database.h"

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/lexical_cast.hpp>
#include <gdcmAnonymizer.h>
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
