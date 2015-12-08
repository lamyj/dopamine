/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/CGetResponse.h>
#include <dcmtkpp/StoreSCU.h>

#include "GetSCP.h"

namespace dopamine
{

namespace services
{

GetSCP
::GetSCP() :
    SCP(), _callback()
{
    // Nothing else.
}

GetSCP
::GetSCP(dcmtkpp::Network *network, dcmtkpp::Association *association) :
    SCP(network, association), _callback()
{
    // Nothing else.
}

GetSCP
::GetSCP(dcmtkpp::Network *network, dcmtkpp::Association *association,
         GetSCP::Callback const & callback) :
    SCP(network, association), _callback()
{
    this->set_callback(callback);
}

GetSCP
::~GetSCP()
{
    // Nothing to do.
}

GetSCP::Callback const &
GetSCP
::get_callback() const
{
    return this->_callback;
}

void
GetSCP
::set_callback(GetSCP::Callback const & callback)
{
    this->_callback = callback;
}

void
GetSCP
::operator()(dcmtkpp::message::Message const & message)
{
    dcmtkpp::message::CGetRequest const request(message);

    dcmtkpp::Value::Integer status = this->_generator->initialize(*this->_association, message);
    if (status != dcmtkpp::message::CGetResponse::Pending)
    {
        // Send Error
        dcmtkpp::message::CGetResponse response(
            request.get_message_id(), status);
        this->_send(response, request.get_affected_sop_class_uid());
    }

    dcmtkpp::StoreSCU scu;
    scu.set_network(this->_network);
    scu.set_association(this->_association);

    while (status == dcmtkpp::message::CGetResponse::Pending)
    {
        try
        {
            status = this->_callback(*this->_association, request, this->_generator);
        }
        catch(dcmtkpp::Exception const & exception)
        {
            status = dcmtkpp::message::CGetResponse::Status::UnableToProcess;
            // Error Comment
            // Error ID
            // Affected SOP Class UID
        }

        if (status == dcmtkpp::message::CGetResponse::Pending)
        {
            // send CSTORE request
            scu.set_affected_sop_class(this->_generator->get().second);
            scu.store(this->_generator->get().second);
        }

        dcmtkpp::message::CGetResponse response(
            request.get_message_id(), status);
        this->_send(response, request.get_affected_sop_class_uid());
    }
}

} // namespace services

} // namespace dopamine
