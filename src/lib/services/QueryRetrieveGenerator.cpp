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
#include "ServicesTools.h"

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
::process()
{
    this->_allow = true;

    if (!this->_isconnected || this->_connection.isFailed())
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

    mongo::BSONObj query_object = this->_bsonquery;

    // Always include the keys for the query level and its higher levels
    if (!this->_bsonquery.hasField("00080052"))
    {
        dopamine::loggerError() << "Cannot find DCM_QueryRetrieveLevel";
        if (this->_service_name == Service_Query)
        {
            return 0xa700; // STATUS_FIND_Refused_OutOfResources
        }
        return 0xa701; // STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches or
                       // STATUS_GET_Refused_OutOfResourcesNumberOfMatches
    }
    this->_query_retrieve_level = this->_bsonquery.getField("00080052").Obj().getField("Value").Array()[0].String();

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
        // retrieve 'Content' field
        fields_builder << "Content" << 1;
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
    if ((this->_query_retrieve_level=="SERIES" ||
         this->_query_retrieve_level=="IMAGE") && !fields_builder.hasField("0020000e"))
    {
        fields_builder << "0020000e" << 1;
    }
    if ((this->_service_name != Service_Query ||
         this->_query_retrieve_level=="IMAGE") &&
        !fields_builder.hasField("00080018"))
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

    // Number of XXX Related Instances (0020,120X)
    std::vector<std::string> tags = {"00201200", "00201202", "00201204",
                                     "00201206", "00201208", "00201209"};
    for (auto tag : tags)
    {
        if (query_object.hasField(tag))
        {
            this->_instance_count_tags.push_back(tag);
        }
    }

    // Modalities in Study (0008,0061)
    this->_convert_modalities_in_study = (this->_service_name == Service_Query &&
                                          query_object.hasField("00080061"));

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

std::string
QueryRetrieveGenerator
::retrieve_dataset_as_string(const mongo::BSONObj &object)
{
    mongo::BSONObj localobject = object;
    if (this->_query_retrieve_level != "IMAGE" &&
        std::find(this->_includefields.begin(),
                  this->_includefields.end(), "00080018") != this->_includefields.end())
    {
        localobject.removeField("00080018");
    }
    return get_dataset_as_string(this->_connection, this->_db_name, localobject);
}

DcmDataset *
QueryRetrieveGenerator
::retrieve_dataset(const mongo::BSONObj &object)
{
    mongo::BSONObj localobject = object;
    if (this->_query_retrieve_level != "IMAGE" &&
        std::find(this->_includefields.begin(),
                  this->_includefields.end(), "00080018") != this->_includefields.end())
    {
        localobject.removeField("00080018");
    }
    return bson_to_dataset(this->_connection, this->_db_name, localobject);
}

std::vector<std::string>
QueryRetrieveGenerator
::get_instance_count_tags() const
{
    return this->_instance_count_tags;
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

int
QueryRetrieveGenerator
::get_maximumResults() const
{
    return this->_maximumResults;
}

void
QueryRetrieveGenerator
::set_skippedResults(int skippedResults)
{
    this->_skippedResults = skippedResults;
}

int
QueryRetrieveGenerator
::get_skippedResults() const
{
    return this->_skippedResults;
}

void
QueryRetrieveGenerator
::set_fuzzymatching(bool fuzzymatching)
{
    this->_fuzzymatching = fuzzymatching;
}

bool
QueryRetrieveGenerator
::get_fuzzymatching() const
{
    return this->_fuzzymatching;
}

unsigned int
QueryRetrieveGenerator
::get_count(std::string const & relatedElement,
            std::string const & ofElement,
            std::string const & value)
{
    mongo::BSONObj object = BSON("distinct" << "datasets" << "key" << relatedElement <<
                                  "query" << BSON(ofElement << value));

    mongo::BSONObj info;
    bool ret = this->_connection.runCommand(this->_db_name,
                                   object, info);

    return info["values"].Array().size();
}

mongo::BSONObj
QueryRetrieveGenerator
::compute_attribute(const std::string &attribute,
                    const std::string &value)
{
    if (attribute == "00080056") // Instance Availability
    {
        return BSON(attribute << BSON("vr" << "CS" << "Value" << BSON_ARRAY("ONLINE")));
    }
    else if (attribute == "00080061") // Modalities in Study
    {
        mongo::BSONObj object = BSON("distinct" << "datasets" << "key" << "00080060.Value" <<
                                      "query" << BSON("0020000d.Value" << value));

        mongo::BSONObj info;
        bool ret = this->_connection.runCommand(this->_db_name,
                                       object, info);

        return BSON(attribute << BSON("vr" << "CS" << "Value" << info["values"]));
    }
    else if (attribute == "00201200") // Number of Patient Related Study
    {
        unsigned int size = this->get_count("0020000d", "00100020.Value", value);

        return BSON(attribute << BSON("vr" << "IS" << "Value" << BSON_ARRAY(size)));
    }
    else if (attribute == "00201202") // Number of Patient Related Series
    {
        unsigned int size = this->get_count("0020000e", "00100020.Value", value);

        return BSON(attribute << BSON("vr" << "IS" << "Value" << BSON_ARRAY(size)));
    }
    else if (attribute == "00201204") // Number of Patient Related Instances
    {
        unsigned int size = this->get_count("00080018", "00100020.Value", value);

        return BSON(attribute << BSON("vr" << "IS" << "Value" << BSON_ARRAY(size)));
    }
    else if (attribute == "00201206") // Number of Study Related Series
    {
        unsigned int size = this->get_count("0020000e", "0020000d.Value", value);

        return BSON(attribute << BSON("vr" << "IS" << "Value" << BSON_ARRAY(size)));
    }
    else if (attribute == "00201208") // Number of Study Related Instances
    {
        unsigned int size = this->get_count("00080018", "0020000d.Value", value);

        return BSON(attribute << BSON("vr" << "IS" << "Value" << BSON_ARRAY(size)));
    }
    else if (attribute == "00201209") // Number of Series Related Instances
    {
        unsigned int size = this->get_count("00080018", "0020000e.Value", value);

        return BSON(attribute << BSON("vr" << "IS" << "Value" << BSON_ARRAY(size)));
    }

    return mongo::BSONObj();
}

} // namespace services

} // namespace dopamine
