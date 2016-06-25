/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <odil/message/CGetRequest.h>
#include <odil/message/CGetResponse.h>

#include "ConverterBSON/bson_converter.h"
#include "core/LoggerPACS.h"
#include "GetGenerator.h"

namespace dopamine
{

namespace services
{

GetGenerator::Pointer
GetGenerator
::New()
{
    return Pointer(new GetGenerator());
}

GetGenerator
::GetGenerator():
    GeneratorPACS()
{
    // Nothing else.
}

GetGenerator
::~GetGenerator()
{
    // Nothing to do.
}

odil::Value::Integer
GetGenerator
::initialize(odil::Association const & association,
             odil::message::Message const & message)
{
    auto const status = GeneratorPACS::initialize(association, message);
    if (status != odil::message::Response::Success)
    {
        return status;
    }

    odil::message::CGetRequest getrequest(message);
    mongo::BSONObj const object = as_bson(getrequest.get_data_set(),
                                          FilterAction::INCLUDE);

    return this->initialize(object);
}

odil::Value::Integer
GetGenerator
::next()
{
    mongo::BSONObj current_bson = this->_cursor->next();

    if (current_bson.isValid() && current_bson.isEmpty())
    {
        // We're done.
        return odil::message::CGetResponse::Success;
    }
    else if (current_bson.hasField("$err"))
    {
        logger_warning() << "An error occured while processing Get operation: "
                         << current_bson.getField("$err").String();
        return odil::message::CGetResponse::ProcessingFailure;
    }
    else
    {
        if (this->_query_retrieve_level != "IMAGE" &&
            std::find(this->_include_fields.begin(),
                      this->_include_fields.end(),
                      "00080018") != this->_include_fields.end())
        {
            current_bson.removeField("00080018");
        }
        auto data_set = this->_connection->get_dataset(current_bson);
        this->_meta_information = data_set.first;
        this->_current_dataset = data_set.second;
    }

    return odil::message::CGetResponse::Pending;
}

odil::Value::Integer
GetGenerator
::initialize(mongo::BSONObj const & request)
{
    auto const status = GeneratorPACS::initialize(request);
    if (status != odil::message::Response::Success)
    {
        return status;
    }

    if (!this->_connection->is_authorized(
                this->_username, odil::message::Message::Command::C_GET_RQ))
    {
        logger_warning() << "User '" << this->_username
                         << "' not allowed to perform Get Operation";
        return odil::message::CGetResponse::RefusedNotAuthorized;
    }

    mongo::BSONObj const constraint = this->_connection->get_constraints(
                this->_username,
                odil::message::Message::Command::C_GET_RQ);

    mongo::BSONObj query_object = request;

    // Always include the keys for the query level and its higher levels
    if (!this->extract_query_retrieve_level(query_object))
    {
        return odil::message::CGetResponse::MissingAttribute;
    }

    // Remove unused elements
    query_object =
            query_object.removeField("00080005"); // SpecificCharacterSet
    query_object =
            query_object.removeField("00080052"); // QueryRetrieveLevel

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

        DicomQueryToMongoQuery function =
                this->_get_query_conversion(match_type);

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

    // retrieve 'Content' field
    fields_builder << "Content" << 1;

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
        this->_query_retrieve_level=="IMAGE") &&
       !fields_builder.hasField("0020000d"))
    {
        fields_builder << "0020000d" << 1;
    }
    if ((this->_query_retrieve_level=="SERIES" ||
         this->_query_retrieve_level=="IMAGE") &&
        !fields_builder.hasField("0020000e"))
    {
        fields_builder << "0020000e" << 1;
    }
    if (!fields_builder.hasField("00080018"))
    {
        fields_builder << "00080018" << 1;
    }

    for (std::string const field : this->_include_fields)
    {
        if (!fields_builder.hasField(field))
        {
            fields_builder << field << 1;
        }
    }

    // Number of XXX Related Instances (0020,120X)
    std::vector<std::string> const tags = {"00201200", "00201202", "00201204",
                                           "00201206", "00201208", "00201209"};
    for (std::string const tag : tags)
    {
        if (query_object.hasField(tag))
        {
            this->_instance_count_tags.push_back(tag);
        }
    }

    // Create Query
    mongo::BSONArrayBuilder finalquerybuilder;
    finalquerybuilder << constraint << db_query.obj();
    mongo::Query const query(BSON("$and" << finalquerybuilder.arr()));

    // Get fields to retrieve
    mongo::BSONObj const fields = fields_builder.obj();

    // Searching into database...
    this->_cursor = this->_connection->get_datasets_cursor(
                query, this->_maximum_results, this->_skipped_results, &fields);

    return odil::message::CGetResponse::Pending;
}

} // namespace services

} // namespace dopamine
