/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "FindGenerator.h"

#include <odil/AssociationParameters.h>
#include <odil/Exception.h>
#include <odil/message/Message.h>
#include <odil/message/Response.h>
#include <odil/registry.h>
#include <odil/SCP.h>
#include <odil/Value.h>

#include "core/LoggerPACS.h"
#include "dbconnection/MongoDBConnection.h"
#include "services/QueryRetrieveGenerator.h"

namespace dopamine
{

namespace services
{

FindGenerator
::FindGenerator(
    odil::AssociationParameters const & parameters,
    MongoDBConnection & db_connection)
: QueryRetrieveGenerator(parameters, db_connection),
    _query_retrieve_level(""), _compute_modalities_in_study(false)
{
    // Nothing else.
}

FindGenerator
::~FindGenerator()
{
    // Nothing to do.
}

void
FindGenerator
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
        query, 0, 0, fields.obj());

    this->_query_retrieve_level = request.get_data_set().as_string(
        odil::registry::QueryRetrieveLevel, 0);
    this->_compute_modalities_in_study = request.get_data_set().has(
        odil::registry::ModalitiesInStudy);
}

void
FindGenerator
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
        logger_warning() << "An error occured while processing Find operation: "
                         << current_bson.getField("$err").String();
        throw odil::SCP::Exception(
            "", odil::message::Response::ProcessingFailure);
    }
    else
    {
        auto header_and_data_set = this->_db_connection.get_dataset(current_bson);
        auto & data_set = header_and_data_set.second;
        data_set.add(
            odil::registry::QueryRetrieveLevel, {this->_query_retrieve_level});

        if (this->_compute_modalities_in_study)
        {
            data_set.remove(odil::registry::Modality);
            std::vector<mongo::BSONElement> const modalities =
                current_bson.getField("modalities_in_study").Array();
            odil::Value::Strings values;
            for(unsigned int i=0; i<modalities.size(); ++i)
            {
                values.push_back(modalities[i].String());
            }
            data_set.add(odil::registry::ModalitiesInStudy, values);
        }

        this->_data_set = data_set;
    }
}

} // namespace services

} // namespace dopamine
