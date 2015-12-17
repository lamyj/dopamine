/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/CMoveResponse.h>
#include <dcmtkpp/StoreSCU.h>

#include "core/ConfigurationPACS.h"
#include "MoveSCP.h"

namespace dopamine
{

namespace services
{

MoveSCP
::MoveSCP() :
    SCP()
{
    // Nothing else.
}

MoveSCP
::MoveSCP(dcmtkpp::Network * network, dcmtkpp::DcmtkAssociation * association) :
    SCP(network, association)
{
    // Nothing else.
}

MoveSCP
::~MoveSCP()
{
    // Nothing to do.
}

void
MoveSCP
::operator()(dcmtkpp::message::Message const & message)
{
    dcmtkpp::message::CMoveRequest const request(message);

    auto status = this->_generator->initialize(*this->_association, message);
    if (status != dcmtkpp::message::CMoveResponse::Pending)
    {
        // Send Error
        dcmtkpp::message::CMoveResponse response(request.get_message_id(),
                                                 status);
        this->_send(response, request.get_affected_sop_class_uid());
        return;
    }

    std::string peer_hostname_port;
    if (!ConfigurationPACS::
            get_instance().peer_for_aetitle(request.get_move_destination(),
                                            peer_hostname_port))
    {
        // Send Error
        dcmtkpp::message::CMoveResponse response(
                request.get_message_id(),
                dcmtkpp::message::CMoveResponse::RefusedMoveDestinationUnknown);
        this->_send(response, request.get_affected_sop_class_uid());
        return;
    }

    dcmtkpp::DcmtkAssociation association;

    association.set_own_ae_title(this->_association->get_own_ae_title());
    std::string peer_host_name =
            peer_hostname_port.substr(0, peer_hostname_port.find(':'));
    association.set_peer_host_name(peer_host_name);
    std::string peer_port =
            peer_hostname_port.substr(peer_hostname_port.find(':')+1);
    association.set_peer_port(atoi(peer_port.c_str()));
    association.set_peer_ae_title(request.get_move_destination());

    // add all negociated presentation contexts
    association.set_presentation_contexts(
                this->_get_all_storage_prensentation_contexts());

    association.associate(*(this->_network));

    dcmtkpp::StoreSCU scu;
    scu.set_network(this->_network);
    scu.set_association(&association);

    try
    {
        scu.echo();
    }
    catch (dcmtkpp::Exception const & exc)
    {
        // Send Error
        dcmtkpp::message::CMoveResponse response(
                    request.get_message_id(),
                    dcmtkpp::message::CMoveResponse::UnableToProcess);
        this->_send(response, request.get_affected_sop_class_uid());
    }

    while (status == dcmtkpp::message::CMoveResponse::Pending)
    {
        try
        {
            if (this->_generator->done())
            {
                status = dcmtkpp::message::CMoveResponse::Success;
            }
            else
            {
                status = this->_generator->next();
            }
        }
        catch(dcmtkpp::Exception const & exception)
        {
            status = dcmtkpp::message::CMoveResponse::Status::UnableToProcess;
            // Error Comment
            // Error ID
            // Affected SOP Class UID
        }

        if (status == dcmtkpp::message::CMoveResponse::Pending)
        {
            // send CSTORE request
            scu.set_affected_sop_class(this->_generator->get().second);
            scu.store(this->_generator->get().second);
        }

        dcmtkpp::message::CMoveResponse response(request.get_message_id(),
                                                 status);
        this->_send(response, request.get_affected_sop_class_uid());
    }

    association.release();
}

std::vector<dcmtkpp::DcmtkAssociation::PresentationContext>
MoveSCP::
_get_all_storage_prensentation_contexts() const
{
    std::vector<dcmtkpp::DcmtkAssociation::PresentationContext>
            presentation_contexts;

    for (auto item : dcmtkpp::registry::uids_dictionary)
    {
        // Attention: cannot add all syntaxes, limit is 127 (odd values of char)
        if (item.second.keyword.find("ImageStorage") != std::string::npos)
        {
            presentation_contexts.push_back(
                dcmtkpp::DcmtkAssociation::PresentationContext(
                    item.first, { dcmtkpp::registry::ImplicitVRLittleEndian }));
        }
    }

    return presentation_contexts;
}

} // namespace services
    
} // namespace dopamine
