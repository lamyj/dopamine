/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmnet/dimse.h>

#include "ConverterBSON/BSONToDataSet.h"
#include "ConverterBSON/DataSetToBSON.h"
#include "ConverterBSON/TagMatch.h"
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
    _query_retrieve_level(""), _convert_modalities_in_study(false)
{
}

Uint16
QueryRetrieveGenerator
::set_query(mongo::BSONObj const & query_dataset)
{
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

    // Look for user authorization
    if ( ! is_authorized(this->_connection, this->_db_name,
                         this->_username, this->_service_name) )
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

    // Build the MongoDB query and query fields from the query dataset.
    mongo::BSONObjBuilder db_query;
    mongo::BSONObjBuilder fields_builder;
    for(mongo::BSONObj::iterator it = query_dataset.begin(); it.more();)
    {
        mongo::BSONElement const element = it.next();
        std::vector<mongo::BSONElement> const array = element.Array();

        // Always include the field in the results
        fields_builder << element.fieldName() << 1;

        std::string const vr = array[0].String();
        mongo::BSONElement const & value = array[1];
        Match::Type const match_type = this->_get_match_type(vr, value);

        DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);
        // Match the array element containing the value
        (this->*function)(std::string(element.fieldName())+".1", vr, value, db_query);
    }

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

    // Handle reduce-related attributes
    std::string reduce_function;
    mongo::BSONObjBuilder initial_builder;

    // Number of XXX Related Instances (0020,120X)
    if(query_dataset.hasField("00201204"))
    {
        this->_instance_count_tag = DCM_NumberOfPatientRelatedInstances;
    }
    else if(query_dataset.hasField("00201208"))
    {
        this->_instance_count_tag = DCM_NumberOfStudyRelatedInstances;
    }
    else if(query_dataset.hasField("00201209"))
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
    if(this->_service_name == Service_Query && query_dataset.hasField("00080061"))
    {
        // Use the Modality attribute
        mongo::BSONElement modalities = query_dataset["00080061"];
        Match::Type const match_type =
            this->_get_match_type("CS", modalities);
        DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);
        (this->*function)("00080060.1", "CS", modalities, db_query);
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
                                            query, 0, 0, &fields);

    return STATUS_Pending;
}

DcmDataset *
QueryRetrieveGenerator
::bson_to_dataset(mongo::BSONObj object)
{
    DcmDataset* dataset = NULL;

    if ( ! object.hasField("location"))
    {
        BSONToDataSet bson2dataset;
        DcmDataset result = bson2dataset(object);
        dataset = new DcmDataset(result);
    }
    else
    {
        std::string const path = object.getField("location").String();
        DcmFileFormat fileformat;
        OFCondition result = fileformat.loadFile(path.c_str());
        if (result.bad())
        {
            loggerError() << "Cannot load dataset '" << path << "': "
                          << result.text();
            return NULL;
        }
        dataset = fileformat.getAndRemoveDataset();
    }

    return dataset;
}

mongo::BSONObj
QueryRetrieveGenerator
::dataset_to_bson(DcmDataset * const dataset)
{
    // Always include the keys for the query level and its higher levels
    OFString ofstring;
    OFCondition condition = dataset->findAndGetOFString(DCM_QueryRetrieveLevel,
                                                        ofstring);
    if (condition.bad())
    {
        dopamine::loggerError() << "Cannot find DCM_QueryRetrieveLevel: "
                                << condition .text();
        return mongo::BSONObj();
    }
    this->_query_retrieve_level = std::string(ofstring.c_str());

    // Convert the dataset to BSON, excluding Query/Retrieve Level.
    DataSetToBSON dataset_to_bson;

    dataset_to_bson.get_filters().push_back(
        std::make_pair(TagMatch::New(DCM_QueryRetrieveLevel),
                       DataSetToBSON::FilterAction::EXCLUDE));
    dataset_to_bson.get_filters().push_back(
        std::make_pair(TagMatch::New(DCM_SpecificCharacterSet),
                       DataSetToBSON::FilterAction::EXCLUDE));
    dataset_to_bson.set_default_filter(DataSetToBSON::FilterAction::INCLUDE);

    mongo::BSONObjBuilder query_builder;
    dataset_to_bson(dataset, query_builder);
    return query_builder.obj();
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

} // namespace services

} // namespace dopamine
