/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/Server.h"

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>

#include <boost/asio.hpp>
#include <mongo/client/dbclient.h>
#include <odil/Association.h>
#include <odil/AssociationAcceptor.h>
#include <odil/EchoSCP.h>
#include <odil/FindSCP.h>
#include <odil/GetSCP.h>
#include <odil/MoveSCP.h>
#include <odil/StoreSCP.h>
#include <odil/SCPDispatcher.h>

#include "dopamine/authentication/AuthenticatorBase.h"
#include "dopamine/AccessControlList.h"
#include "dopamine/archive/echo.h"
#include "dopamine/archive/GetDataSetGenerator.h"
#include "dopamine/archive/MoveDataSetGenerator.h"
#include "dopamine/archive/QueryDataSetGenerator.h"
#include "dopamine/archive/Storage.h"
#include "dopamine/archive/store.h"
#include "dopamine/logging.h"
#include "dopamine/utils.h"

namespace dopamine
{

Server
::Server(
    mongo::DBClientConnection & connection,
    std::string const & database, std::string const & bulk_database,
    uint16_t port, authentication::AuthenticatorBase const & authenticator)
: _connection(connection), _database(database), _bulk_database(bulk_database),
  _port(port), _authenticator(authenticator), _acl(connection, database),
  _storage(connection, database, bulk_database), _is_running(false)
{
    // Nothing else.
}

Server
::~Server()
{
    // Nothing to do.
}

void
Server
::run()
{
    this->_is_running = true;
    while(this->_is_running)
    {
        this->_association = std::make_shared<odil::Association>();
        try
        {
            this->_association->receive_association(
                boost::asio::ip::tcp::v4(), this->_port,
                std::bind(
                    Server::_acceptor, std::placeholders::_1,
                    std::ref(this->_authenticator)));
        }
        catch(odil::AssociationRejected const &)
        {
            DOPAMINE_LOG(DEBUG)
                << "Incoming association from "
                << this->_association->get_transport().get_socket()->remote_endpoint().address()
                << "/"
                << this->_association->get_parameters().get_calling_ae_title()
                << "rejected";
            // FIXME: close ?
            this->_association = nullptr;
            continue;
        }
        catch(std::exception const & e)
        {
            DOPAMINE_LOG(ERROR)
                << "Failed receiving association: "
                << e.what() << " (" << typeid(e).name() << ")";
            // FIXME: close ?
            this->_association = nullptr;
            continue;
        }

        DOPAMINE_LOG(INFO)
            << "Association received from "
            << this->_association->get_transport().get_socket()->remote_endpoint().address()
            << " ("
            << this->_association->get_negotiated_parameters().get_calling_ae_title()
            << " -> "
            << this->_association->get_negotiated_parameters().get_called_ae_title()
            << ")";
        DOPAMINE_LOG(DEBUG) << "Negotiated presentation contexts: ";
        for(auto const & pc: this->_association->get_negotiated_parameters().get_presentation_contexts())
        {
            DOPAMINE_LOG(DEBUG)
                << get_uid_name(pc.abstract_syntax) << " / "
                << get_uid_name(pc.transfer_syntaxes[0]) << " "
                << (pc.scu_role_support?"SCU":"")
                << (pc.scu_role_support&&pc.scp_role_support?"/":"")
                << (pc.scp_role_support?"SCP":"");
        }

        auto dispatcher = this->_get_dispatcher(*this->_association);

        bool done = false;
        while(!done)
        {
            try
            {
                dispatcher.dispatch();
            }
            catch(odil::AssociationReleased const &)
            {
                DOPAMINE_LOG(INFO)
                    << "Association released from "
                    << this->_association->get_transport().get_socket()->remote_endpoint().address();
                done = true;
            }
            catch(odil::AssociationAborted const &)
            {
                DOPAMINE_LOG(INFO)
                    << "Association aborted from "
                    << this->_association->get_transport().get_socket()->remote_endpoint().address();
                done = true;
            }
            catch(std::exception const & e)
            {
                DOPAMINE_LOG(ERROR)
                    << "Failed dispatching messages: " << e.what();
                done = true;
            }
        }

        this->_association = nullptr;
    }
}

void
Server
::shutdown()
{
    this->_is_running = false;
    this->_association->get_transport().close();
}

odil::AssociationParameters
Server
::_acceptor(
    odil::AssociationParameters const & input,
    authentication::AuthenticatorBase const & authenticator)
{
    bool authenticated;
    try
    {
        authenticated = authenticator(input);
    }
    catch(std::exception const & e)
    {
        throw odil::AssociationRejected(
            odil::Association::RejectedTransient,
            odil::Association::ULServiceProvderPresentationRelatedFunction,
            odil::Association::NoReasonGiven,
            std::string("Authentication failure: ")+e.what());
    }

    if(!authenticated)
    {
        throw odil::AssociationRejected(
            odil::Association::RejectedTransient,
            odil::Association::ULServiceUser,
            odil::Association::NoReasonGiven, "Invalid credentials");
    }

    odil::AssociationParameters output;

    output.set_called_ae_title(input.get_called_ae_title());
    output.set_calling_ae_title(input.get_calling_ae_title());

    auto presentation_contexts = input.get_presentation_contexts();
    for(auto & presentation_context: presentation_contexts)
    {
        presentation_context.transfer_syntaxes =
            { presentation_context.transfer_syntaxes[0] };
        presentation_context.result =
            odil::AssociationParameters::PresentationContext::Result::Acceptance;
    }
    output.set_presentation_contexts(presentation_contexts);

    output.set_maximum_length(input.get_maximum_length());

       return output;
   };

odil::SCPDispatcher
Server
::_get_dispatcher(odil::Association & association) const
{
   odil::SCPDispatcher dispatcher(association);

   auto echo_scp = std::make_shared<odil::EchoSCP>(
       association, std::bind(
           archive::echo, std::ref(this->_connection), this->_acl,
           association.get_negotiated_parameters(), std::placeholders::_1));
   dispatcher.set_scp(odil::message::Message::Command::C_ECHO_RQ, echo_scp);

   auto find_scp = std::make_shared<odil::FindSCP>(
       association, std::make_shared<archive::QueryDataSetGenerator>(
           this->_connection, this->_acl, this->_database,
           association.get_negotiated_parameters()));
   dispatcher.set_scp(odil::message::Message::Command::C_FIND_RQ, find_scp);

   auto get_scp = std::make_shared<odil::GetSCP>(
       association, std::make_shared<archive::GetDataSetGenerator>(
           this->_connection, this->_acl,
           this->_database, this->_bulk_database,
           association.get_negotiated_parameters()));
   dispatcher.set_scp(odil::message::Message::Command::C_GET_RQ, get_scp);

   auto move_scp = std::make_shared<odil::MoveSCP>(
       association, std::make_shared<archive::MoveDataSetGenerator>(
           this->_connection, this->_acl,
           this->_database, this->_bulk_database,
           association.get_negotiated_parameters()));
   dispatcher.set_scp(odil::message::Message::Command::C_MOVE_RQ, move_scp);

   auto store_scp = std::make_shared<odil::StoreSCP>(
        association, std::bind(
            archive::store, this->_acl, association.get_negotiated_parameters(),
            this->_storage, std::placeholders::_1));
   dispatcher.set_scp(odil::message::Message::Command::C_STORE_RQ, store_scp);

   return dispatcher;
}


} // namespace dopamine
