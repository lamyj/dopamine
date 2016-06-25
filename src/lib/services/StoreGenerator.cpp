/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <odil/message/CStoreRequest.h>
#include <odil/message/CStoreResponse.h>

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

odil::Value::Integer
StoreGenerator
::initialize(odil::Association const & association,
             odil::message::Message const & message)
{
    auto const status = GeneratorPACS::initialize(association, message);
    if (status != odil::message::Response::Success)
    {
        return status;
    }

    odil::message::CStoreRequest storerequest(message);

    this->_peer_ae_title = association.get_parameters().get_calling_ae_title();

    auto dataset = storerequest.get_data_set();
    return this->initialize(dataset);
}

odil::Value::Integer
StoreGenerator
::next()
{
    // all work doing into Initilization, nothing to do.
    return odil::message::Response::Success;
}

odil::Value::Integer
StoreGenerator
::initialize(odil::DataSet const & dataset)
{
    mongo::BSONObj const object;
    auto const status = GeneratorPACS::initialize(object);
    if (status != odil::message::Response::Success)
    {
        return status;
    }

    if (!this->_connection->is_authorized(
                this->_username, odil::message::Message::Command::C_STORE_RQ))
    {
        logger_warning() << "User '" << this->_username
                         << "' not allowed to perform Store Operation";
        return odil::message::CStoreResponse::RefusedNotAuthorized;
    }

    // Dataset should not be empty
    if (dataset.empty())
    {
        return odil::message::CStoreResponse::ErrorCannotUnderstand;
    }

    // Should have SOP Instance UID
    if (!dataset.has(odil::registry::SOPInstanceUID))
    {
        return odil::message::CStoreResponse::InvalidObjectInstance;
    }

    // Get the SOP Instance UID
    std::string const sopinstanceuid =
            dataset.as_string(odil::registry::SOPInstanceUID)[0];

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
        return odil::message::CStoreResponse::ProcessingFailure;
    }

    // If the command correctly executed and database entries match
    if (info["n"].Double() > 0)
    {
        // We already have this SOP Instance UID, do not store it
        logger_warning() << "Store: SOP Instance UID already register";
        return odil::message::CStoreResponse::Pending; // Nothing to do
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
