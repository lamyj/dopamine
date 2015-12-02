/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "EchoSCP.h"

#include <dcmtkpp/message/CEchoResponse.h>

namespace dopamine
{

namespace services
{

EchoSCP
::EchoSCP() :
    dcmtkpp::SCP(), _callback()
{
    // Nothing else.
}

EchoSCP
::EchoSCP(dcmtkpp::Network * network, dcmtkpp::Association * association) :
    dcmtkpp::SCP(network, association), _callback()
{
    // Nothing else.
}

EchoSCP
::EchoSCP(dcmtkpp::Network * network, dcmtkpp::Association * association,
          EchoSCP::Callback const & callback) :
    dcmtkpp::SCP(network, association), _callback()
{
    this->set_callback(callback);
}

EchoSCP::~EchoSCP()
{
    // Nothing to do.
}

EchoSCP::Callback const &
EchoSCP
::get_callback() const
{
    return this->_callback;
}

void
EchoSCP
::set_callback(EchoSCP::Callback const & callback)
{
    this->_callback = callback;
}

void
EchoSCP
::operator()(dcmtkpp::message::Message const & message)
{
    dcmtkpp::message::CEchoRequest const request(message);

    dcmtkpp::Value::Integer status = dcmtkpp::message::CEchoResponse::Success;

    try
    {
        status = this->_callback(*this->_association, request);
    }
    catch(dcmtkpp::Exception const & exception)
    {
        status = dcmtkpp::message::CEchoResponse::MistypedArgument;
        // Error Comment
        // Error ID
        // Affected SOP Class UID
    }

    dcmtkpp::message::CEchoResponse response(
        request.get_message_id(), status,
        request.get_affected_sop_class_uid());

    this->_send(response, request.get_affected_sop_class_uid());
}

} // namespace services

} // namespace dopamine
