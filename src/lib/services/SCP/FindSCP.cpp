/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/CFindResponse.h>

#include "FindSCP.h"

namespace dopamine
{

namespace services
{

FindSCP
::FindSCP() :
    SCP()
{
    // Nothing else.
}

FindSCP
::FindSCP(dcmtkpp::Network * network, dcmtkpp::DcmtkAssociation * association) :
    SCP(network, association)
{
    // Nothing else.
}

FindSCP
::~FindSCP()
{
    // Nothing to do.
}

void
FindSCP
::operator()(dcmtkpp::message::Message const & message)
{
    dcmtkpp::message::CFindRequest const request(message);

    auto status = this->_generator->initialize(*this->_association, message);
    if (status != dcmtkpp::message::CFindResponse::Pending)
    {
        // Send Error
        dcmtkpp::message::CFindResponse response(request.get_message_id(),
                                                 status);
        this->_send(response, request.get_affected_sop_class_uid());
    }

    while (status == dcmtkpp::message::CFindResponse::Pending)
    {
        try
        {
            if (this->_generator->done())
            {
                status = dcmtkpp::message::CFindResponse::Success;
            }
            else
            {
                status = this->_generator->next();
            }
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
            dcmtkpp::message::CFindResponse response(request.get_message_id(),
                                                     status);
            this->_send(response, request.get_affected_sop_class_uid());
        }
    }
}

} // namespace services

} // namespace dopamine
