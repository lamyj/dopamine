/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcmetinf.h>
#include <dcmtk/dcmnet/dimse.h>

#include "core/ConfigurationPACS.h"
#include "core/Hashcode.h"
#include "core/LoggerPACS.h"
#include "services/ServicesTools.h"
#include "StoreGenerator.h"

namespace dopamine
{

namespace services
{

StoreGenerator
::StoreGenerator(const std::string &username):
    Generator(username), _destination_path("")
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
        loggerWarning() << "Could not connect to database: " << this->_db_name;
        return STATUS_STORE_Refused_OutOfResources;
    }

    // Look for user authorization
    this->_allow = is_authorized(this->_connection, this->_db_name,
                                 this->_username, Service_Store);
    if ( ! this->_allow )
    {
        loggerWarning() << "User '" << this->_username << "' not allowed to perform "
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
    OFCondition condition = this->_dataset->findAndGetOFStringArray(DCM_SOPInstanceUID,
                                                                    sopinstanceuid);
    if (condition.bad())
    {
        return STATUS_STORE_Refused_OutOfResources;
    }
    mongo::BSONObj group_command =
            BSON("count" << "datasets" << "query"
                 << BSON("00080018.Value" << BSON_ARRAY(sopinstanceuid.c_str())));

    mongo::BSONObj info;
    bool ret = this->_connection.runCommand(this->_db_name, group_command, info, 0);

    // If an error occurred
    if (!ret || info["ok"].Double() != 1)
    {
        loggerWarning() << "Could not connect to database: " << this->_db_name;
        return STATUS_STORE_Refused_OutOfResources;
    }

    // If the command correctly executed and database entries match
    if (info["ok"].Double() == 1 && info["n"].Double() > 0)
    {
        // We already have this SOP Instance UID, do not store it
        loggerWarning() << "Store: SOP Instance UID already register";
        return STATUS_Pending; // Nothing to do
    }

    if (insert_dataset(this->_connection, this->_db_name,
                       this->_username, this->_dataset,
                       this->_callingaptitle) != NO_ERROR)
    {
        return STATUS_STORE_Refused_OutOfResources;
    }

    // Everything OK
    return STATUS_Pending;
}

void
StoreGenerator
::set_callingaptitle(const std::string &callingaptitle)
{
    this->_callingaptitle = callingaptitle;
}

std::string
StoreGenerator
::get_callingaptitle() const
{
    return this->_callingaptitle;
}

void
StoreGenerator
::create_destination_path(const mongo::BSONObj &query_dataset)
{
    // Compute the destination filename
    boost::gregorian::date const today(
        boost::gregorian::day_clock::universal_day());

    std::string study_instance_uid =
            query_dataset.getField("0020000d").Obj().getField("Value").Array()[0].String(); // DCM_StudyInstanceUID
    std::string series_instance_uid =
            query_dataset.getField("0020000e").Obj().getField("Value").Array()[0].String(); // DCM_SeriesInstanceUID
    std::string sop_instance_uid =
            query_dataset.getField("00080018").Obj().getField("Value").Array()[0].String(); // DCM_SOPInstanceUID

    std::string const study_hash = dopamine::hashcode::hashToString
            (dopamine::hashcode::hashCode(study_instance_uid));
    std::string const series_hash = dopamine::hashcode::hashToString
            (dopamine::hashcode::hashCode(series_instance_uid));
    std::string const sop_instance_hash = dopamine::hashcode::hashToString
            (dopamine::hashcode::hashCode(sop_instance_uid));

    boost::filesystem::path const destination =
        boost::filesystem::path(
            ConfigurationPACS::get_instance().GetValue("dicom.storage_path"))
            /boost::lexical_cast<std::string>(today.year())
            /boost::lexical_cast<std::string>(today.month().as_number())
            /boost::lexical_cast<std::string>(today.day())
            /study_hash/series_hash/sop_instance_hash;

    _destination_path = destination.string();
}

bool
StoreGenerator
::is_dataset_allowed_for_storage(mongo::BSONObj const & dataset)
{
    // Retrieve user's Rights
    mongo::BSONObj constraint =
            get_constraint_for_user(this->_connection, this->_db_name,
                                    this->_username, Service_Store);

    if (constraint.isEmpty())
    {
        // No constraint
        return true;
    }

    // Compare with input dataset

    // constraint is a Logical OR array
    for (auto itemor : constraint["$or"].Array())
    {
        bool result = true;
        // Each item is a Logical AND array
        for (auto itemand : itemor["$and"].Array())
        {
            // Foreach object
            for(mongo::BSONObj::iterator it=itemand.Obj().begin(); it.more();)
            {
                mongo::BSONElement const element = it.next();

                // Retrieve DICOM field name
                std::string name(element.fieldName());
                name = name.substr(0, 8);

                // Check if input dataset as this field
                if (!dataset.hasField(name))
                {
                    result = false;
                    break;
                }
                else
                {
                    // Compare the field's values
                    auto array = dataset.getField(name).Obj().getField("Value").Array();
                    for(auto itarray = array.begin(); itarray != array.end(); ++itarray)
                    {
                        mongo::BSONElement const element2 = *itarray;
                        std::string valuestr = bsonelement_to_string(element2);

                        if (element.type() == mongo::BSONType::RegEx)
                        {
                            std::string regex(element.regex());
                            if (!boost::regex_match(valuestr.c_str(), boost::regex(regex.c_str())))
                            {
                                result = false;
                                break;
                            }
                        }
                        else
                        {
                            std::string elementstr = bsonelement_to_string(element);
                            if (valuestr != elementstr)
                            {
                                result = false;
                                break;
                            }
                        }
                    }
                    if (result == false)
                    {
                        break;
                    }
                }
            }
        }

        // Stop if find dataset match with one condition
        if (result)
        {
            return true;
        }
        // else continue
    }

    return false;
}

} // namespace services

} // namespace dopamine
