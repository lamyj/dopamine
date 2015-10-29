/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <dcmtkpp/registry.h>
#include <dcmtkpp/Response.h>

#include "core/ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "services/ServicesTools.h"
#include "StoreGenerator.h"

namespace dopamine
{

namespace services
{

StoreGenerator
::StoreGenerator(const std::string &username):
    Generator(username), _calling_aptitle(""), _destination_path("")
{
    // Nothing to do
}

StoreGenerator
::~StoreGenerator()
{
    // Nothing to do
}

Uint16
StoreGenerator
::process()
{
    // Look for database connection
    if (this->_connection.isFailed())
    {
        logger_warning() << "Could not connect to database: " << this->_db_name;
        return 0xa700; //dcmtkpp::Response::Todo_Refused;
    }

    // Look for user authorization
    this->_allow = is_authorized(this->_connection, this->_db_name,
                                 this->_username, Service_Store);
    if ( ! this->_allow )
    {
        logger_warning() << "User '" << this->_username
                         << "' not allowed to perform "
                         << Service_Store;
        return 0xa700; //dcmtkpp::Response::Todo_Refused;
    }

    // Dataset should not be empty
    if (this->_dataset.empty())
    {
        return 0xa700; //dcmtkpp::Response::Todo_Refused;
    }

    if (!this->_dataset.has(dcmtkpp::registry::SOPInstanceUID))
    {
        return 0xa700; //dcmtkpp::Response::Todo_Refused;
    }

    // Get the SOP Instance UID
    std::string const sopinstanceuid =
            this->_dataset.as_string(dcmtkpp::registry::SOPInstanceUID)[0];

    mongo::BSONObj const group_command =
            BSON("count" << "datasets" << "query"
                 << BSON("00080018.Value" <<
                         BSON_ARRAY(sopinstanceuid)));

    mongo::BSONObj info;
    bool ret = this->_connection.runCommand(this->_db_name,
                                            group_command, info, 0);

    // If an error occurred
    if (!ret || info["ok"].Double() != 1)
    {
        logger_warning() << "Could not connect to database: "
                         << this->_db_name;
        return 0xa700; //dcmtkpp::Response::Todo_Refused;
    }

    // If the command correctly executed and database entries match
    if (info["ok"].Double() == 1 && info["n"].Double() > 0)
    {
        // We already have this SOP Instance UID, do not store it
        logger_warning() << "Store: SOP Instance UID already register";
        return dcmtkpp::Response::Pending; // Nothing to do
    }

    if (insert_dataset(this->_connection, this->_db_name,
                       this->_username, this->_dataset,
                       this->_calling_aptitle) != NO_ERROR)
    {
        return 0xa700; //dcmtkpp::Response::Todo_Refused;
    }

    // Everything OK
    return dcmtkpp::Response::Pending;
}

void
StoreGenerator
::set_calling_aptitle(std::string const & callingaptitle)
{
    this->_calling_aptitle = callingaptitle;
}

std::string
StoreGenerator
::get_calling_aptitle() const
{
    return this->_calling_aptitle;
}

} // namespace services

} // namespace dopamine
