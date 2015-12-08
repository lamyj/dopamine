/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "FindSCP.h"

#include <dcmtkpp/message/CFindResponse.h>

namespace dopamine
{

namespace services
{

FindSCP
::FindSCP() :
    SCP(), _callback()
{
    // Nothing else.
}

FindSCP
::FindSCP(dcmtkpp::Network * network, dcmtkpp::Association * association) :
    SCP(network, association), _callback()
{
    // Nothing else.
}

FindSCP
::FindSCP(dcmtkpp::Network * network, dcmtkpp::Association * association,
          FindSCP::Callback const & callback) :
    SCP(network, association), _callback()
{
    this->set_callback(callback);
}

FindSCP::~FindSCP()
{
    // Nothing to do.
}

FindSCP::Callback const &
FindSCP
::get_callback() const
{
    return this->_callback;
}

void
FindSCP
::set_callback(FindSCP::Callback const & callback)
{
    this->_callback = callback;
}

void
FindSCP
::operator()(dcmtkpp::message::Message const & message)
{
    dcmtkpp::message::CFindRequest const request(message);

    dcmtkpp::Value::Integer status = this->_generator->initialize(*this->_association, message);
    if (status != dcmtkpp::message::CFindResponse::Pending)
    {
        // Send Error
        dcmtkpp::message::CFindResponse response(
            request.get_message_id(), status);
        this->_send(response, request.get_affected_sop_class_uid());
    }

    while (status == dcmtkpp::message::CFindResponse::Pending)
    {
        try
        {
            status = this->_callback(*this->_association, request, this->_generator);
        }
        catch(dcmtkpp::Exception const & exception)
        {
            status = dcmtkpp::message::CFindResponse::Status::UnableToProcess;
            // Error Comment
            // Error ID
            // Affected SOP Class UID
        }

        if (status == dcmtkpp::message::CFindResponse::Pending)
        {
            dcmtkpp::message::CFindResponse response(
                        request.get_message_id(), status,
                        this->_generator->get().second);
            this->_send(response, request.get_affected_sop_class_uid());
        }
        else
        {
            dcmtkpp::message::CFindResponse response(
                request.get_message_id(), status);
            this->_send(response, request.get_affected_sop_class_uid());
        }
    }
}

} // namespace services

} // namespace dopamine
