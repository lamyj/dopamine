/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <map>
#include <memory>

#include <dcmtkpp/Exception.h>
#include <dcmtkpp/ServiceRole.h>
#include <dcmtkpp/Value.h>

#include "LoggerPACS.h"
#include "SCPDispatcher.h"

namespace dopamine
{

SCPDispatcher
::SCPDispatcher()
: dcmtkpp::ServiceRole()
{
    // Nothing else.
}

SCPDispatcher
::SCPDispatcher(dcmtkpp::Network * network, dcmtkpp::Association * association)
: dcmtkpp::ServiceRole(network, association)
{
    // Nothing else.
}

SCPDispatcher
::~SCPDispatcher()
{
    // Nothing to do.
}

bool
SCPDispatcher
::has_scp(dcmtkpp::Value::Integer command) const
{
    auto const it = this->_providers.find(command);
    return (it != this->_providers.end());
}

services::SCP &
SCPDispatcher
::get_scp(dcmtkpp::Value::Integer command)
{
    auto const it = this->_providers.find(command);
    if(it == this->_providers.end())
    {
        throw dcmtkpp::Exception("No such provider");
    }
    return *(it->second);
}

bool
SCPDispatcher
::dispatch()
{
    bool receive_shutdown = false;
    // check if we have negotiated the private "shutdown" SOP Class
    if (0 != ASC_findAcceptedPresentationContextID(this->_association->get_association(),
                                                   UID_PrivateShutdownSOPClass))
    {
        dopamine::logger_info()
                << "Shutting down server ... "
                << "(negotiated private \"shut down\" SOP class)";
        receive_shutdown = true;
    }
    else
    {
        auto const message = this->_receive();

        logger_info() << "Received message " << std::hex << message.get_command_field();
        logger_info() << message.get_command_set().as_string(dcmtkpp::registry::AffectedSOPClassUID)[0];
        logger_info() << "Message " << (message.has_data_set()?(message.get_data_set().empty()?"has an empty ":"has a "):"has no ") << "data set";

        try
        {
            auto & scp = this->get_scp(message.get_command_field());
            scp.set_network(this->get_network());
            scp.set_association(this->get_association());
            scp(message);
            scp.set_association(NULL);
            scp.set_network(NULL);
        }
        catch(dcmtkpp::Exception const & e)
        {
            logger_error() << e.what();
        }
    }

    try
    {
        this->_receive();
        // If we get here, the SCP did not finish processing.
        throw dcmtkpp::Exception("SCP did not finish processing");
    }
    catch(dcmtkpp::Exception const & exception)
    {
        if(exception.get_condition() == DUL_PEERREQUESTEDRELEASE)
        {
            logger_debug() << "Association release";
            this->_association->drop();
            this->set_association(NULL);
        }
        else
        {
            throw;
        }
    }

    return !receive_shutdown;
}

}
