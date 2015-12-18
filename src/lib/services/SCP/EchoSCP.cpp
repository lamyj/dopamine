/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/CEchoResponse.h>

#include "EchoSCP.h"

namespace dopamine
{

namespace services
{

EchoSCP
::EchoSCP() :
    SCP()
{
    // Nothing else.
}

EchoSCP
::EchoSCP(dcmtkpp::Network * network, dcmtkpp::DcmtkAssociation * association) :
    SCP(network, association)
{
    // Nothing else.
}

EchoSCP
::~EchoSCP()
{
    // Nothing to do.
}

void
EchoSCP
::operator()(dcmtkpp::message::Message const & message)
{
    dcmtkpp::message::CEchoRequest const request(message);

    auto status = this->_generator->initialize(*this->_association, message);
    if (status != dcmtkpp::message::CEchoResponse::Pending)
    {
        // Send Error
        dcmtkpp::message::CEchoResponse response(
                    request.get_message_id(), status,
                    request.get_affected_sop_class_uid());
        this->_association->send(response, request.get_affected_sop_class_uid());
    }
    else
    {
        try
        {
            if (this->_generator->done())
            {
                status = dcmtkpp::message::CEchoResponse::Success;
            }
            else
            {
                status = this->_generator->next();
            }
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

        this->_association->send(response, request.get_affected_sop_class_uid());
    }
}

} // namespace services

} // namespace dopamine
