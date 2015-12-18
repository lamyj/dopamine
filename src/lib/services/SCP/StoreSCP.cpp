/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/CStoreResponse.h>

#include "StoreSCP.h"

namespace dopamine
{

namespace services
{

StoreSCP
::StoreSCP() :
    SCP()
{
    // Nothing else.
}

StoreSCP
::StoreSCP(dcmtkpp::Network *network, dcmtkpp::DcmtkAssociation *association) :
    SCP(network, association)
{
    // Nothing else.
}

StoreSCP
::~StoreSCP()
{
    // Nothing to do.
}

void
StoreSCP
::operator()(dcmtkpp::message::Message const & message)
{
    dcmtkpp::message::CStoreRequest const request(message);

    auto status = this->_generator->initialize(*this->_association, message);
    if (status == dcmtkpp::message::CStoreResponse::Pending)
    {
        try
        {
            if (this->_generator->done())
            {
                status = dcmtkpp::message::CStoreResponse::Success;
            }
            else
            {
                status = this->_generator->next();
            }
        }
        catch(dcmtkpp::Exception const & exception)
        {
            status = dcmtkpp::message::CStoreResponse::ProcessingFailure;
            // Error Comment
            // Error ID
            // Affected SOP Class UID
        }
    }

    dcmtkpp::message::CStoreResponse const response(request.get_message_id(),
                                                    status);
    this->_association->send(response, request.get_affected_sop_class_uid());
}

} // namespace services

} // namespace dopamine
