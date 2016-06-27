/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "GetGenerator.h"

#include <odil/AssociationParameters.h>
#include <odil/message/Request.h>
#include <odil/message/Response.h>
#include <odil/SCP.h>

#include "core/LoggerPACS.h"
#include "dbconnection/MongoDBConnection.h"
#include "services/QueryRetrieveGenerator.h"

namespace dopamine
{

namespace services
{

GetGenerator
::GetGenerator(
    odil::AssociationParameters const & parameters,
    MongoDBConnection & db_connection)
: QueryRetrieveGenerator(parameters, db_connection)
{
    // Nothing else.
}

GetGenerator
::~GetGenerator()
{
    // Nothing to do.
}

void
GetGenerator
::initialize(odil::message::Request const & request)
{
    // Build the MongoDB query and query fields from the DICOM query
    mongo::BSONArrayBuilder terms;
    mongo::BSONObjBuilder fields;
    this->_get_query_and_fields(request, terms, fields);

    // Build the full query (query terms and constraints)
    auto const constraints = this->_db_connection.get_constraints(
        this->_username,
        odil::message::Message::Command::Type(request.get_command_field()));
    mongo::Query const query(
        BSON_ARRAY("$and" << constraints << BSON_ARRAY("$and" << terms.arr())));

    this->_cursor = this->_db_connection.get_datasets_cursor(
        query, 0, 0, BSON("Content" << 1));
}

void
GetGenerator
::next()
{
    mongo::BSONObj current_bson = this->_cursor->next();

    if(current_bson.isValid() && current_bson.isEmpty())
    {
        // We're done.
        return;
    }
    else if(current_bson.hasField("$err"))
    {
        logger_warning() << "An error occured while processing Get operation: "
                         << current_bson.getField("$err").String();
        throw odil::SCP::Exception(
            "", odil::message::Response::ProcessingFailure);
    }
    else
    {
        auto header_and_data_set = this->_db_connection.get_dataset(current_bson);
        this->_data_set = header_and_data_set.second;
    }
}

} // namespace services

} // namespace dopamine
