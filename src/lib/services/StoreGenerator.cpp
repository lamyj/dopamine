/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

/* make sure OS specific configuration is included first */
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcmetinf.h>
#include <dcmtk/dcmnet/dimse.h>

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
    if (this->_db_information.connection.isFailed())
    {
        logger_warning() << "Could not connect to database: "
            << this->_db_information.db_name;
        return STATUS_STORE_Refused_OutOfResources;
    }

    // Look for user authorization
    this->_allow = is_authorized(this->_db_information,
                                 this->_username, Service_Store);
    if ( ! this->_allow )
    {
        logger_warning() << "User '" << this->_username
                         << "' not allowed to perform "
                         << Service_Store;
        return STATUS_STORE_Refused_OutOfResources;
    }

    // Dataset should not be empty
    if (this->_dataset == NULL)
    {
        return STATUS_STORE_Refused_OutOfResources;
    }

    // Get the SOP Instance UID
    OFString sopinstanceuid;
    OFCondition condition =
            this->_dataset->findAndGetOFStringArray(DCM_SOPInstanceUID,
                                                    sopinstanceuid);
    if (condition.bad())
    {
        return STATUS_STORE_Refused_OutOfResources;
    }
    mongo::BSONObj const group_command =
            BSON("count" << "datasets" << "query"
                 << BSON("00080018.Value" <<
                         BSON_ARRAY(sopinstanceuid.c_str())));

    mongo::BSONObj info;
    bool ret = this->_db_information.connection.runCommand(
        this->_db_information.db_name, group_command, info, 0);

    // If an error occurred
    if (!ret || info["ok"].Double() != 1)
    {
        logger_warning() << "Could not connect to database: "
                         << this->_db_information.db_name;
        return STATUS_STORE_Refused_OutOfResources;
    }

    // If the command correctly executed and database entries match
    if (info["ok"].Double() == 1 && info["n"].Double() > 0)
    {
        // We already have this SOP Instance UID, do not store it
        logger_warning() << "Store: SOP Instance UID already register";
        return STATUS_Pending; // Nothing to do
    }

    if (insert_dataset(this->_db_information,
                       this->_username, this->_dataset,
                       this->_calling_aptitle) != NO_ERROR)
    {
        return STATUS_STORE_Refused_OutOfResources;
    }

    // Everything OK
    return STATUS_Pending;
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
