/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/CStoreRequest.h>
#include <dcmtkpp/message/CStoreResponse.h>

#include "core/LoggerPACS.h"
#include "StoreGenerator.h"

namespace dopamine
{

namespace services
{

StoreGenerator::Pointer
StoreGenerator
::New()
{
    return Pointer(new StoreGenerator());
}

StoreGenerator
::StoreGenerator():
    GeneratorPACS(), _peer_ae_title("")
{
    // Nothing else.
}

StoreGenerator
::~StoreGenerator()
{
    // Nothing to do.
}

dcmtkpp::Value::Integer
StoreGenerator
::initialize(dcmtkpp::Association const & association,
             dcmtkpp::message::Message const & message)
{
    auto const status = GeneratorPACS::initialize(association, message);
    if (status != dcmtkpp::message::Response::Success)
    {
        return status;
    }

    dcmtkpp::message::CStoreRequest storerequest(message);

    this->_peer_ae_title = association.get_peer_ae_title();

    auto dataset = storerequest.get_data_set();
    return this->initialize(dataset);
}

dcmtkpp::Value::Integer
StoreGenerator
::next()
{
    // all work doing into Initilization, nothing to do.
    return dcmtkpp::message::Response::Success;
}

dcmtkpp::Value::Integer
StoreGenerator
::initialize(dcmtkpp::DataSet const & dataset)
{
    mongo::BSONObj const object;
    auto const status = GeneratorPACS::initialize(object);
    if (status != dcmtkpp::message::Response::Success)
    {
        return status;
    }

    if (!this->_connection->is_authorized(
                this->_username, dcmtkpp::message::Message::Command::C_STORE_RQ))
    {
        logger_warning() << "User '" << this->_username
                         << "' not allowed to perform Store Operation";
        return dcmtkpp::message::CStoreResponse::RefusedNotAuthorized;
    }

    // Dataset should not be empty
    if (dataset.empty())
    {
        return dcmtkpp::message::CStoreResponse::ErrorCannotUnderstand;
    }

    // Should have SOP Instance UID
    if (!dataset.has(dcmtkpp::registry::SOPInstanceUID))
    {
        return dcmtkpp::message::CStoreResponse::InvalidObjectInstance;
    }

    // Get the SOP Instance UID
    std::string const sopinstanceuid =
            dataset.as_string(dcmtkpp::registry::SOPInstanceUID)[0];

    mongo::BSONObj const command =
            BSON("count" << "datasets" << "query"
                 << BSON("00080018.Value" <<
                         BSON_ARRAY(sopinstanceuid)));

    mongo::BSONObj info;
    bool result = this->_connection->run_command(command, info);

    // If an error occurred
    if (!result)
    {
        logger_warning() << "Could not connect to database";
        return dcmtkpp::message::CStoreResponse::ProcessingFailure;
    }

    // If the command correctly executed and database entries match
    if (info["n"].Double() > 0)
    {
        // We already have this SOP Instance UID, do not store it
        logger_warning() << "Store: SOP Instance UID already register";
        return dcmtkpp::message::CStoreResponse::Pending; // Nothing to do
    }

    return this->_connection->insert_dataset(this->_username,
                                             dataset,
                                             this->_peer_ae_title);
}

std::string
StoreGenerator
::get_peer_ae_title() const
{
    return this->_peer_ae_title;
}

} // namespace services

} // namespace dopamine
