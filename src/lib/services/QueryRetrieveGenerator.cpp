/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmnet/dimse.h>

#include "core/LoggerPACS.h"
#include "QueryRetrieveGenerator.h"

namespace dopamine
{

namespace services
{

QueryRetrieveGenerator
::QueryRetrieveGenerator(const std::string &username,
                         const std::string &service_name):
    Generator(username), _service_name(service_name),
    _query_retrieve_level(""), _convert_modalities_in_study(false),
    _maximumResults(0), _skippedResults(0), _fuzzymatching(false)
{
    // Nothing to do
}

Uint16
QueryRetrieveGenerator
::set_query(mongo::BSONObj const & query_dataset)
{
    this->_allow = true;

    if (this->_connection.isFailed())
    {
        loggerWarning() << "Could not connect to database: " << this->_db_name;
        if (this->_service_name == Service_Query)
        {
            return 0xa700; // STATUS_FIND_Refused_OutOfResources
        }
        return 0xa701; // STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches or
                       // STATUS_GET_Refused_OutOfResourcesNumberOfMatches
    }

    this->_allow = is_authorized(this->_connection, this->_db_name,
                                 this->_username, this->_service_name);

    // Look for user authorization
    if ( ! this->_allow )
    {
        loggerWarning() << "User '" << this->_username << "' not allowed to perform "
                        << this->_service_name;
        if (this->_service_name == Service_Query)
        {
            return 0xa700; // STATUS_FIND_Refused_OutOfResources
        }
        return 0xa701; // STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches or
                       // STATUS_GET_Refused_OutOfResourcesNumberOfMatches
    }

    mongo::BSONObj constraint = get_constraint_for_user(this->_connection,
                                                        this->_db_name,
                                                        this->_username,
                                                        this->_service_name);

    mongo::BSONObj query_object = query_dataset;

    // Always include the keys for the query level and its higher levels
    if (!query_dataset.hasField("00080052"))
    {
        dopamine::loggerError() << "Cannot find DCM_QueryRetrieveLevel";
        if (this->_service_name == Service_Query)
        {
            return 0xa700; // STATUS_FIND_Refused_OutOfResources
        }
        return 0xa701; // STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches or
                       // STATUS_GET_Refused_OutOfResourcesNumberOfMatches
    }
    this->_query_retrieve_level = query_dataset.getField("00080052").Obj().getField("Value").Array()[0].String();

    // Remove unused elements
    query_object = query_object.removeField("00080005"); // DCM_SpecificCharacterSet
    query_object = query_object.removeField("00080052"); // DCM_QueryRetrieveLevel

    // Build the MongoDB query and query fields from the query dataset.
    mongo::BSONObjBuilder db_query;
    mongo::BSONObjBuilder fields_builder;

    mongo::BSONArrayBuilder arraybuilder;
    for(mongo::BSONObj::iterator it = query_object.begin(); it.more();)
    {
        mongo::BSONObjBuilder db_query_sub;

        mongo::BSONElement const element = it.next();
        mongo::BSONObj const bsonobj = element.Obj();

        // Always include the field in the results
        fields_builder << element.fieldName() << 1;
        std::string const vr = bsonobj.getField("vr").String();

        std::string fieldtoget = "Value";
        if (vr == "OB" || vr == "OF" || vr == "OW" || vr == "UN")
        {
            fieldtoget = "InlineBinary";
        }

        mongo::BSONElement const & value = bsonobj.getField(fieldtoget);
        Match::Type const match_type = this->_get_match_type(vr, value);

        DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);

        std::stringstream field;
        field << std::string(element.fieldName()) << "." << fieldtoget;
        if (vr == "PN")
        {
            field << ".Alphabetic";
        }
        // Match the array element containing the value
        (this->*function)(field.str(), vr, value, db_query_sub);

        arraybuilder << db_query_sub.obj();
    }
    db_query << "$and" << arraybuilder.arr();

    if (this->_service_name != Service_Query)
    {
        // retrieve 'location' field
        fields_builder << "location" << 1;
    }

    // Always include Specific Character Set in results.
    if(!fields_builder.hasField("00080005"))
    {
        fields_builder << "00080005" << 1;
    }

    if(!fields_builder.hasField("00100020"))
    {
        fields_builder << "00100020" << 1;
    }
    if((this->_query_retrieve_level=="STUDY" ||
        this->_query_retrieve_level=="SERIES" ||
        this->_query_retrieve_level=="IMAGE") && !fields_builder.hasField("0020000d"))
    {
        fields_builder << "0020000d" << 1;
    }
    if((this->_query_retrieve_level=="SERIES" ||
        this->_query_retrieve_level=="IMAGE") && !fields_builder.hasField("0020000e"))
    {
        fields_builder << "0020000e" << 1;
    }
    if(this->_query_retrieve_level=="IMAGE" && !fields_builder.hasField("00080018"))
    {
        fields_builder << "00080018" << 1;
    }

    for (unsigned int i = 0; i < this->_includefields.size(); ++i)
    {
        if (!fields_builder.hasField(this->_includefields.at(i)))
        {
            fields_builder << this->_includefields.at(i) << 1;
        }
    }

    // Handle reduce-related attributes
    std::string reduce_function;
    mongo::BSONObjBuilder initial_builder;

    // Number of XXX Related Instances (0020,120X)
    if(query_object.hasField("00201204"))
    {
        this->_instance_count_tag = DCM_NumberOfPatientRelatedInstances;
    }
    else if(query_object.hasField("00201208"))
    {
        this->_instance_count_tag = DCM_NumberOfStudyRelatedInstances;
    }
    else if(query_object.hasField("00201209"))
    {
        this->_instance_count_tag = DCM_NumberOfSeriesRelatedInstances;
    }
    else
    {
        this->_instance_count_tag = DCM_UndefinedTagKey;
    }
    if (this->_instance_count_tag != DCM_UndefinedTagKey)
    {
        reduce_function += "result.instance_count+=1;";
        initial_builder << "instance_count" << 0;
    }

    // Modalities in Study (0008,0061)
    if(this->_service_name == Service_Query && query_object.hasField("00080061"))
    {
        // Use the Modality attribute
        mongo::BSONElement modalities = query_object["00080061"];
        Match::Type const match_type =
            this->_get_match_type("CS", modalities);
        DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);
        (this->*function)("00080060.Value", "CS", modalities, db_query);
        fields_builder << "00080060" << 1;
        reduce_function +=
            "if(result.modalities_in_study.indexOf(current[\"00080060\"][1])==-1) "
            "{ result.modalities_in_study.push(current[\"00080060\"][1]); }";
        initial_builder << "modalities_in_study" << mongo::BSONArrayBuilder().arr();
        this->_convert_modalities_in_study = true;
    }
    else
    {
        this->_convert_modalities_in_study = false;
    }

    // Create Query
    mongo::BSONArrayBuilder finalquerybuilder;
    finalquerybuilder << constraint << db_query.obj();
    mongo::Query query(BSON("$and" << finalquerybuilder.arr()));

    // Get fields to retrieve
    mongo::BSONObj const fields = fields_builder.obj();

    // Searching into database...
    this->_cursor = this->_connection.query(this->_db_name + ".datasets",
                                            query, this->_maximumResults,
                                            this->_skippedResults, &fields);

    return STATUS_Pending;
}

DcmTagKey
QueryRetrieveGenerator
::get_instance_count_tag() const
{
    return this->_instance_count_tag;
}

bool
QueryRetrieveGenerator
::get_convert_modalities_in_study() const
{
    return this->_convert_modalities_in_study;
}

std::string
QueryRetrieveGenerator
::get_query_retrieve_level() const
{
    return this->_query_retrieve_level;
}

void
QueryRetrieveGenerator
::set_includefields(std::vector<std::string> includefields)
{
    this->_includefields = includefields;
}

void
QueryRetrieveGenerator
::set_maximumResults(int maximumResults)
{
    this->_maximumResults = maximumResults;
}

void
QueryRetrieveGenerator
::set_skippedResults(int skippedResults)
{
    this->_skippedResults = skippedResults;
}

void
QueryRetrieveGenerator
::set_fuzzymatching(bool fuzzymatching)
{
    this->_fuzzymatching = fuzzymatching;
}

} // namespace services

} // namespace dopamine
