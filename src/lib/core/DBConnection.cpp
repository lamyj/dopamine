/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "DBConnection.h"
#include "ExceptionPACS.h"
#include "LoggerPACS.h"

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
    _db_name(""), _db_host(""), _db_port("")
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
::Initialize(const std::string &db_name,
             const std::string &db_host,
             const std::string &db_port,
             std::vector<std::string> indexeslist)
{
    this->_db_name = db_name;
    this->_db_host = db_host;
    this->_db_port = db_port;
    this->_indexeslist.clear();

    for (auto currentIndex : indexeslist)
    {
        DcmTag dcmtag;
        OFCondition ret = DcmTag::findTagFromName(currentIndex.c_str(), dcmtag);

        if (ret.good())
        {
            this->_indexeslist.push_back(dcmtag);
        }
    }
}

void
DBConnection
::connect()
{
    if (this->_db_name == "" || this->_db_host == "" || this->_db_port == "")
    {
        throw ExceptionPACS("DBConnection not initialize.");
    }

    // Try to connect database
    // Disconnect is automatic when it calls the destructors
    std::string errormsg = "";
    if ( ! this->_connection.connect(mongo::HostAndPort(this->_db_host + ":" + this->_db_port),
                                     errormsg) )
    {
        std::stringstream stream;
        stream << "DBConnection::connect error: " << errormsg;
        throw ExceptionPACS(stream.str());
    }
    research_pacs::loggerDebug() << "DBconnection::connect OK";
    
    // Create indexes
    std::string const datasets = this->_db_name + ".datasets";

    for (DcmTag currentIndex : this->_indexeslist)
    {
        // convert Uint16 => string XXXXYYYY
        char buffer[9];
        snprintf(buffer, 9, "%04x%04x", currentIndex.getGroup(), currentIndex.getElement());

        std::stringstream stream;
        stream << "\"" << buffer << "\"";

        this->_connection.ensureIndex
            (
                datasets,
                BSON(stream.str() << 1),
                false,
                std::string(currentIndex.getTagName())
            );
    }
}

bool 
DBConnection
::checkUserAuthorization(UserIdentityNegotiationSubItemRQ & userIdentNeg,
                         T_DIMSE_Command command)
{
    std::string lcurrentUser = "";
    
    // Only available for Identity Type: User or User/Pasword
    if (userIdentNeg.getIdentityType() == ASC_USER_IDENTITY_USER ||
        userIdentNeg.getIdentityType() == ASC_USER_IDENTITY_USER_PASSWORD)
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
        
        if (lusername == lcurrentUser || lusername == "*")
        {
            std::vector<int> operations;
            mongo::BSONObjIterator fields(p.getObjectField("authorizedAction"));
            while(fields.more()) {
                operations.push_back(fields.next().numberInt());
            }
            
            for (int liter = 0 ; liter < operations.size() ; liter++)
            {
                // User authorized
                if (command == T_DIMSE_Command(operations[liter]))
                {
                    return true;
                }
            }
        }
    }
            
    return false;
}
    
} // namespace research_pacs
