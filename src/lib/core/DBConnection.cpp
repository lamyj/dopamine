/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "ConfigurationPACS.h"
#include "DBConnection.h"

namespace research_pacs
{
    
DBConnection * DBConnection::_instance = NULL;

DBConnection &
DBConnection
::get_instance()
{
    if(DBConnection::_instance == NULL)
    {
        DBConnection::_instance = new DBConnection();
    }
    return *DBConnection::_instance;
}

DBConnection
::DBConnection():
    _db_name("")
{
    // nothing to do
}

DBConnection
::~DBConnection()
{
    // nothing to do
}

void
DBConnection
::connect()
{
    this->_db_name = ConfigurationPACS::get_instance().GetValue("database.dbname");
    std::string const hostname = ConfigurationPACS::get_instance().GetValue("database.hostname");
    std::string const port = ConfigurationPACS::get_instance().GetValue("database.port");
    
    this->_connection.connect(hostname + ":" + port);
    
    std::string const datasets = this->_db_name + ".datasets";
    this->_connection.ensureIndex(
        datasets, BSON("\"00080018\"" << 1), false, "SOP Instance UID");
    this->_connection.ensureIndex(
        datasets, BSON("\"00100010\"" << 1), false, "Patient's Name");
    this->_connection.ensureIndex(
        datasets, BSON("\"00100020\"" << 1), false, "Patient ID");

    this->_connection.ensureIndex(
        datasets, BSON("\"0020000e\"" << 1), false, "Series Instance UID");
    this->_connection.ensureIndex(
        datasets, BSON("\"0008103e\"" << 1), false, "Series Description");

    this->_connection.ensureIndex(
        datasets, BSON("\"0020000d\"" << 1), false, "Study Instance UID");
    this->_connection.ensureIndex(
        datasets, BSON("\"00081030\"" << 1), false, "Study Description");
}

bool 
DBConnection
::checkUserAuthorization(UserIdentityNegotiationSubItemRQ & userIdentNeg,
                         T_DIMSE_Command command)
{
    std::string lcurrentUser;
    
    // Only available for Identity Type: User/Pasword
    if (userIdentNeg.getIdentityType() == ASC_USER_IDENTITY_USER_PASSWORD)
    {
        // Get user name
        char * user; Uint16 user_length;
        userIdentNeg.getPrimField(user, user_length);
        // user is not NULL-terminated
        lcurrentUser = std::string(user, user_length);
        delete [] user;
    }
    
    // Get authorization
    mongo::auto_ptr< mongo::DBClientCursor> cursor = 
        this->_connection.query(this->_db_name+"."+"authorization",  
                                mongo::BSONObj());

    while (cursor->more())
    {
        mongo::BSONObj p = cursor->next();
        
        std::string lusername = p.getStringField("username");
        
        if (lusername == lcurrentUser)
        {
            std::vector<int> operations;
            mongo::BSONObjIterator fields(p.getObjectField("authorizedAction"));
            while(fields.more()) {
                operations.push_back(fields.next().numberInt());
            }
            
            for (int liter = 0 ; liter < operations.size() ; liter++)
            {
                // User authorized
                if ((Uint32)command == (Uint32)operations[liter])
                {
                    return true;
                }
            }
        }
    }
            
    return false;
}
    
} // namespace research_pacs
