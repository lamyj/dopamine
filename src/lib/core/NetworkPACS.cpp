/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "authenticator/AuthenticatorCSV.h"
#include "authenticator/AuthenticatorLDAP.h"
#include "authenticator/AuthenticatorNone.h"
#include "core/ConfigurationPACS.h"
#include "core/ExceptionPACS.h"
#include "core/LoggerPACS.h"
#include "core/NetworkPACS.h"
#include "core/SCPDispatcher.h"
#include "dbconnection/MongoDBInformation.h"
#include "services/EchoGenerator.h"
#include "services/FindGenerator.h"
#include "services/GetGenerator.h"
#include "services/MoveGenerator.h"
#include "services/StoreGenerator.h"
#include "services/SCP/EchoSCP.h"
#include "services/SCP/FindSCP.h"
#include "services/SCP/GetSCP.h"
#include "services/SCP/MoveSCP.h"
#include "services/SCP/StoreSCP.h"

namespace dopamine
{

NetworkPACS * NetworkPACS::_instance = NULL;

NetworkPACS &
NetworkPACS
::get_instance()
{
    if(NetworkPACS::_instance == NULL)
    {
        NetworkPACS::_instance = new NetworkPACS();
    }
    return *NetworkPACS::_instance;
}

void
NetworkPACS
::delete_instance()
{
    if (NetworkPACS::_instance != NULL)
    {
        delete NetworkPACS::_instance;
    }
    NetworkPACS::_instance = NULL;
}

NetworkPACS
::NetworkPACS():
    _authenticator(NULL), _is_running(false)
{
    this->_create_authenticator();

    // Get configuration for Database connection
    MongoDBInformation db_information;
    std::string db_host = "";
    int db_port = -1;
    std::vector<std::string> indexeslist;
    ConfigurationPACS::get_instance().get_database_configuration(db_information,
                                                                 db_host,
                                                                 db_port,
                                                                 indexeslist);

    // Create connection with Database
    MongoDBConnection connection(db_information, db_host, db_port, indexeslist);

    // Try to connect
    if (!connection.connect())
    {
        throw ExceptionPACS("cannot connect to database");
    }

    int port = std::atoi(ConfigurationPACS::
                         get_instance().get_value("dicom.port").c_str());

    this->_network = dcmtkpp::Network(NET_ACCEPTORREQUESTOR, port, 30);
    this->_network.initialize();

    if (!this->_network.is_initialized())
    {
        throw ExceptionPACS("cannot initialize network");
    }
}

NetworkPACS
::~NetworkPACS()
{
    this->_network.drop();

    if (this->_authenticator != NULL)
    {
        delete this->_authenticator;
    }
}

void NetworkPACS::_create_authenticator()
{
    ConfigurationPACS const & instance = ConfigurationPACS::get_instance();
    std::string const type = instance.get_value("authenticator.type");
    if(type == "CSV")
    {
        this->_authenticator = new authenticator::AuthenticatorCSV
            (instance.get_value("authenticator.filepath"));
    }
    else if (type == "LDAP")
    {
        this->_authenticator = new authenticator::AuthenticatorLDAP
            (
                instance.get_value("authenticator.ldap_server"),
                instance.get_value("authenticator.ldap_bind_user"),
                instance.get_value("authenticator.ldap_base"),
                instance.get_value("authenticator.ldap_filter")
            );
    }
    else if (type == "None")
    {
        this->_authenticator = new authenticator::AuthenticatorNone();
    }
    else
    {
        std::stringstream stream;
        stream << "Unknown authentication type " << type;
        throw ExceptionPACS(stream.str());
    }
}

void NetworkPACS::run()
{
    this->_is_running = true;

    SCPDispatcher dispatcher;
    dispatcher.set_network(&this->_network);

    // SCP
    services::EchoSCP echoscp;
    echoscp.set_generator(services::EchoGenerator::New());
    dispatcher.set_scp(dcmtkpp::message::Message::Command::C_ECHO_RQ, echoscp);

    services::FindSCP findscp;
    findscp.set_generator(services::FindGenerator::New());
    dispatcher.set_scp(dcmtkpp::message::Message::Command::C_FIND_RQ, findscp);

    services::GetSCP getscp;
    getscp.set_generator(services::GetGenerator::New());
    dispatcher.set_scp(dcmtkpp::message::Message::Command::C_GET_RQ, getscp);

    services::MoveSCP movescp;
    movescp.set_generator(services::MoveGenerator::New());
    movescp.set_own_aetitle(ConfigurationPACS::get_instance().get_value("dicom.ae_title"));
    dispatcher.set_scp(dcmtkpp::message::Message::Command::C_MOVE_RQ, movescp);

    services::StoreSCP storescp;
    storescp.set_generator(services::StoreGenerator::New());
    dispatcher.set_scp(dcmtkpp::message::Message::Command::C_STORE_RQ, storescp);

    // Loop Processing
    while(this->_is_running)
    {
        logger_debug() << "Network is waiting for association";

        // Waiting for association
        if (this->_network.is_association_waiting(TIMEOUT))
        {
            dcmtkpp::DcmtkAssociation association;
            dispatcher.set_association(&association);
            try
            {
                auto authenticator_ = [this](dcmtkpp::DcmtkAssociation const & assoc)->bool { return (*this->_authenticator)(assoc); };
                association.receive(this->_network, authenticator_, {"*"}, true);
            }
            catch (dcmtkpp::Exception const & exception)
            {
                logger_error() << "Failed to receive association: "
                               << exception.what();
                continue;
            }

            time_t t = time(NULL);
            logger_info() << "Association Received ("
                          << association.get_peer_host_name()
                          << ":" << association.get_peer_ae_title()
                          << " -> "
                          << association.get_own_ae_title() << ") "
                          << ctime(&t);

            this->_is_running = dispatcher.dispatch();

            dispatcher.set_association(NULL);
        }
    }
}

bool
NetworkPACS
::is_running() const
{
    return this->_is_running;
}

} // namespace dopamine
