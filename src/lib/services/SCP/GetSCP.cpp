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
    SCP()
{
    // Nothing else.
}

GetSCP
::GetSCP(dcmtkpp::Network *network, dcmtkpp::DcmtkAssociation *association) :
    SCP(network, association)
{
    // Nothing else.
}

GetSCP
::~GetSCP()
{
    // Nothing to do.
}

void
GetSCP
::operator()(dcmtkpp::message::Message const & message)
{
    dcmtkpp::message::CGetRequest const request(message);

    auto status = this->_generator->initialize(*this->_association, message);
    if (status != dcmtkpp::message::CGetResponse::Pending)
    {
        // Send Error
        dcmtkpp::message::CGetResponse response(request.get_message_id(),
                                                status);
        this->_send(response, request.get_affected_sop_class_uid());
    }

    dcmtkpp::StoreSCU scu;
    scu.set_network(this->_network);
    scu.set_association(this->_association);

    while (status == dcmtkpp::message::CGetResponse::Pending)
    {
        try
        {
            if (this->_generator->done())
            {
                status = dcmtkpp::message::CGetResponse::Success;
            }
            else
            {
                status = this->_generator->next();
            }
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

        dcmtkpp::message::CGetResponse response(request.get_message_id(),
                                                status);
        this->_send(response, request.get_affected_sop_class_uid());
    }
}

} // namespace services

} // namespace dopamine
