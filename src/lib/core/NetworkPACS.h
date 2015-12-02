/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _95305138_fbe7_4b3a_99d8_9f73013477fd
#define _95305138_fbe7_4b3a_99d8_9f73013477fd

#include <dcmtkpp/Association.h>
#include <dcmtkpp/Network.h>

#include <mongo/client/dbclient.h>

#include "authenticator/AuthenticatorBase.h"

namespace dopamine
{

int const TIMEOUT = 1000;

class NetworkPACS
{
public:
    /**
     * Create (if not exist) and return an unique instance of NetworkPACS
     * @return unique instance of NetworkPACS
     */
    static NetworkPACS & get_instance();

    /**
     * Remove the unique instance of NetworkPACS
     */
    static void delete_instance();

    /// Destroy the Network
    virtual ~NetworkPACS();

    /// While loop to listen the network
    void run();

    /**
     * Stop running after the next received association or time out
     */
    void stop_running();

    bool is_running() const;

protected:

private:
    /// Create an instance of NetworkPACS and initialize the network
    NetworkPACS();

    /// Unique Instance
    static NetworkPACS * _instance;

    /// Network for listening/sending Requests and Responses
    dcmtkpp::Network _network;

    /// Database connection
    mongo::DBClientConnection _connection;

    /// Database name
    std::string _db_name;

    /// Authenticator manager
    authenticator::AuthenticatorBase * _authenticator;

    /// flag indicating Network is running
    bool _is_running;

    /// Initialize the authenticator manager
    void _create_authenticator();

};

} // namespace dopamine

#endif // _95305138_fbe7_4b3a_99d8_9f73013477fd
