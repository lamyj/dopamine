/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <odil/message/CEchoResponse.h>

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

odil::Value::Integer
EchoGenerator
::initialize(odil::Association const & association,
             odil::message::Message const & message)
{
    auto const status = GeneratorPACS::initialize(association, message);
    if (status != odil::message::Response::Success)
    {
        return status;
    }

    mongo::BSONObj query_object;
    return this->initialize(query_object);
}

odil::Value::Integer
EchoGenerator
::initialize(mongo::BSONObj const & request)
{
    auto const status = GeneratorPACS::initialize(request);
    if (status != odil::message::Response::Success)
    {
        return status;
    }

    // Look for user authorization
    if ( ! this->_connection->is_authorized(
             this->_username, odil::message::Message::Command::C_ECHO_RQ) )
    {
        logger_warning() << "User '" << this->_username
                         << "' not allowed to perform Echo Operation";
        return odil::message::CEchoResponse::RefusedNotAuthorized;
    }

    return odil::message::CEchoResponse::Pending;
}

odil::Value::Integer
EchoGenerator
::next()
{
    // Nothing to do.
    return odil::message::CEchoResponse::Success;
}

} // namespace services

} // namespace dopamine
