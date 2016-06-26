/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

//#include <odil/EchoSCP.h>
#include <odil/FindSCP.h>
//#include <odil/GetSCP.h>
//#include <odil/MoveSCP.h>
//#include <odil/StoreSCP.h>
#include <odil/SCPDispatcher.h>

#include "authenticator/AuthenticatorCSV.h"
#include "authenticator/AuthenticatorLDAP.h"
#include "authenticator/AuthenticatorNone.h"
#include "core/ConfigurationPACS.h"
#include "core/ExceptionPACS.h"
#include "core/LoggerPACS.h"
#include "core/NetworkPACS.h"
#include "dbconnection/MongoDBInformation.h"
//#include "services/EchoGenerator.h"
#include "services/FindGenerator.h"
//#include "services/GetGenerator.h"
//#include "services/MoveGenerator.h"
//#include "services/StoreGenerator.h"

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
::NetworkPACS()
: _authenticator(NULL), _is_running(false)
{
    this->_create_authenticator();

    // Get configuration for Database connection
    MongoDBInformation db_information;
    std::string db_host = "";
    int db_port = -1;
    std::vector<std::string> indexeslist;
    ConfigurationPACS::get_instance().get_database_configuration(
        db_information, db_host, db_port, indexeslist);

    // Create connection with Database
    this->_db_connection = MongoDBConnection(
        db_information, db_host, db_port, indexeslist);

    // Try to connect
    if(!this->_db_connection.connect())
    {
        throw ExceptionPACS("cannot connect to database");
    }
}

NetworkPACS
::~NetworkPACS()
{
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

    // Loop Processing
    while(this->_is_running)
    {
        logger_debug() << "Waiting for incoming association";

        auto const port = std::stoul(
            ConfigurationPACS::get_instance().get_value("dicom.port"));
        odil::Association association;
        try
        {
            association.receive_association(
                boost::asio::ip::tcp::v4(), port,
                [this](odil::AssociationParameters const & parameters)
                {
                    if(!(*this->_authenticator)(parameters))
                    {
                        throw odil::AssociationRejected(1, 2, 1);
                    }
                    return odil::default_association_acceptor(parameters);
                }
            );
        }
        catch(odil::Exception const & exception)
        {
            logger_error() << "Failed to receive association: "
                           << exception.what();
            continue;
        }

        logger_debug() << "Association received";


        //auto echo_scp = std::make_shared<odil::EchoSCP>(association, echo);
        auto find_scp = std::make_shared<odil::FindSCP>(
            association,
            std::make_shared<services::FindGenerator>(
                association.get_parameters(), this->_db_connection));
        //auto store_scp = std::make_shared<odil::StoreSCP>(association, store);

        odil::SCPDispatcher dispatcher(association);
        //dispatcher.set_scp(odil::message::Message::Command::C_ECHO_RQ, echo_scp);
        dispatcher.set_scp(odil::message::Message::Command::C_FIND_RQ, find_scp);
        //dispatcher.set_scp(
        //    odil::message::Message::Command::C_STORE_RQ, store_scp);

        bool done = false;
        while(!done)
        {
            try
            {
                dispatcher.dispatch();
            }
            catch(odil::AssociationReleased const &)
            {
                logger_debug() << "Peer released association";
                done = true;
            }
            catch(odil::AssociationAborted const & e)
            {
                logger_debug()
                    << "Peer aborted association, "
                    << "source: " << int(e.source) << ", "
                    << "reason: " << int(e.reason);
                done = true;
            }
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
