/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/CStoreResponse.h>
#include <dcmtkpp/Value.h>

#include "StoreSCP.h"

namespace dopamine
{

namespace services
{

StoreSCP
::StoreSCP() :
    SCP(), _callback()
{
    // Nothing else.
}

StoreSCP
::StoreSCP(dcmtkpp::Network *network, dcmtkpp::Association *association) :
    SCP(network, association), _callback()
{
    // Nothing else.
}

StoreSCP
::StoreSCP(dcmtkpp::Network *network, dcmtkpp::Association *association,
               StoreSCP::Callback const & callback) :
    SCP(network, association), _callback()
{
    this->set_callback(callback);
}

StoreSCP
::~StoreSCP()
{
    // Nothing to do.
}

StoreSCP::Callback const &
StoreSCP
::get_callback() const
{
    return this->_callback;
}

void
StoreSCP
::set_callback(StoreSCP::Callback const & callback)
{
    this->_callback = callback;
}

void
StoreSCP
::operator()(dcmtkpp::message::Message const & message)
{
    dcmtkpp::message::CStoreRequest const request(message);

    dcmtkpp::Value::Integer status = this->_generator->initialize(*this->_association, message);
    if (status == dcmtkpp::message::CStoreResponse::Pending)
    {
        try
        {
            status = this->_callback(*this->_association, request, this->_generator);
        }
        catch(dcmtkpp::Exception const & exception)
        {
            status = dcmtkpp::message::CStoreResponse::ProcessingFailure;
            // Error Comment
            // Error ID
            // Affected SOP Class UID
        }
    }

    dcmtkpp::message::CStoreResponse const response(request.get_message_id(), status);
    this->_send(response, request.get_affected_sop_class_uid());
}

} // namespace services

} // namespace dopamine
