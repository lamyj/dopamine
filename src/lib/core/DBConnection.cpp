/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include "ConfigurationPACS.h"
#include "DBConnection.h"
#include "ExceptionPACS.h"
#include "LoggerPACS.h"

namespace dopamine
{

DBConnection
::DBConnection():
    _db_name("")
{
    // Get all indexes
    std::string indexlist = ConfigurationPACS::get_instance().GetValue("database.indexlist");
    std::vector<std::string> indexeslist;
    boost::split(indexeslist, indexlist, boost::is_any_of(";"));

    this->_db_name = ConfigurationPACS::get_instance().GetValue("database.dbname");
    std::string db_host = ConfigurationPACS::get_instance().GetValue("database.hostname");
    std::string db_port = ConfigurationPACS::get_instance().GetValue("database.port");

    if (this->_db_name == "" || db_host == "" || db_port == "")
    {
        throw ExceptionPACS("DBConnection not initialize.");
    }

    // Try to connect database
    // Disconnect is automatic when it calls the destructors
    std::string errormsg = "";
    if ( ! this->_connection.connect(mongo::HostAndPort(db_host + ":" + db_port),
                                     errormsg) )
    {
        std::stringstream stream;
        stream << "DBConnection::connect error: " << errormsg;
        throw ExceptionPACS(stream.str());
    }
    dopamine::loggerDebug() << "DBconnection::connect OK";

    // Create indexes
    std::string const datasets = this->_db_name + ".datasets";

    for (auto currentIndex : indexeslist)
    {
        DcmTag dcmtag;
        OFCondition ret = DcmTag::findTagFromName(currentIndex.c_str(), dcmtag);

        if (ret.good())
        {
            // convert Uint16 => string XXXXYYYY
            char buffer[9];
            snprintf(buffer, 9, "%04x%04x", dcmtag.getGroup(), dcmtag.getElement());

            std::stringstream stream;
            stream << "\"" << buffer << "\"";

            this->_connection.ensureIndex
                (
                    datasets,
                    BSON(stream.str() << 1),
                    false,
                    std::string(dcmtag.getTagName())
                );
        }
    }
}

DBConnection
::~DBConnection()
{
    // nothing to do
}

bool 
DBConnection
::checkUserAuthorization(UserIdentityNegotiationSubItemRQ & userIdentNeg,
                         T_DIMSE_Command command)
{
    std::string lcurrentUser = "";
    
    // Only available for Identity Type: User or User/Pasword
    if ((&userIdentNeg != NULL) &&
        (userIdentNeg.getIdentityType() == ASC_USER_IDENTITY_USER ||
         userIdentNeg.getIdentityType() == ASC_USER_IDENTITY_USER_PASSWORD))
    {
        // Get user name
        char * user; Uint16 user_length;
        userIdentNeg.getPrimField(user, user_length);
        // user is not NULL-terminated
        lcurrentUser = std::string(user, user_length);
        delete [] user;
    }

    mongo::BSONObjBuilder fields_builder;
    fields_builder << "principal_name" << BSON("$in" << BSON_ARRAY("*" << lcurrentUser))
                   << "service" << BSON("$in" << BSON_ARRAY(Service_All << DBConnection::DIMSE_Command_to_Service(command)));

    mongo::BSONObj group_command = BSON("count" << "authorization" << "query" << fields_builder.obj());

    mongo::BSONObj info;
    this->_connection.runCommand(this->get_db_name(), group_command, info, 0);

    // If the command correctly executed and database entries match
    if (info["ok"].Double() == 1 && info["n"].Double() > 0)
    {
        return true;
    }

    // Not allowed
    return false;
}

std::string DBConnection::DIMSE_Command_to_Service(T_DIMSE_Command command)
{
    switch (command)
    {
    case DIMSE_C_ECHO_RQ:
    {
        return Service_Echo;
    }
    case DIMSE_C_STORE_RQ:
    {
        return Service_Store;
    }
    case DIMSE_C_FIND_RQ:
    {
        return Service_Query;
    }
    case DIMSE_C_GET_RQ:
    case DIMSE_C_MOVE_RQ:
    {
        return Service_Retrieve;
    }
    default:
        break;
    }

    return "";
}

std::string DBConnection::WebService_to_Service(const std::string &webservice)
{
    if (webservice == "QUIDO")
    {
        return Service_Query;
    }
    else if (webservice == "WADO")
    {
        return Service_Retrieve;
    }
    else if (webservice == "STOW")
    {
        return Service_Store;
    }

    return "";
}

} // namespace dopamine
