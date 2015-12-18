/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/CEchoResponse.h>

#include "core/LoggerPACS.h"
#include "EchoGenerator.h"

namespace dopamine
{

namespace services
{

EchoGenerator::Pointer
EchoGenerator
::New()
{
    return Pointer(new EchoGenerator());
}

EchoGenerator
::EchoGenerator():
    GeneratorPACS() // base class initialisation
{
    // Nothing else.
}

EchoGenerator
::~EchoGenerator()
{
    // Nothing to do.
}

dcmtkpp::Value::Integer
EchoGenerator
::initialize(dcmtkpp::DcmtkAssociation const & association,
             dcmtkpp::message::Message const & message)
{
    auto const status = GeneratorPACS::initialize(association, message);
    if (status != dcmtkpp::message::Response::Success)
    {
        return status;
    }

    mongo::BSONObj query_object;
    return this->initialize(query_object);
}

dcmtkpp::Value::Integer
EchoGenerator
::initialize(mongo::BSONObj const & request)
{
    auto const status = GeneratorPACS::initialize(request);
    if (status != dcmtkpp::message::Response::Success)
    {
        return status;
    }

    // Look for user authorization
    if ( ! this->_connection->is_authorized(
             this->_username, dcmtkpp::message::Message::Command::C_ECHO_RQ) )
    {
        logger_warning() << "User '" << this->_username
                         << "' not allowed to perform Echo Operation";
        return dcmtkpp::message::CEchoResponse::RefusedNotAuthorized;
    }

    return dcmtkpp::message::CEchoResponse::Pending;
}

dcmtkpp::Value::Integer
EchoGenerator
::next()
{
    // Nothing to do.
    return dcmtkpp::message::CEchoResponse::Success;
}

} // namespace services

} // namespace dopamine
