/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _13a8d4a4_4144_4910_b54a_702ae291eac2
#define _13a8d4a4_4144_4910_b54a_702ae291eac2

#include <cstdint>
#include <string>

#include <mongo/client/dbclient.h>
#include <odil/Association.h>
#include <odil/SCPDispatcher.h>

#include "dopamine/authentication/AuthenticatorBase.h"
#include "dopamine/AccessControlList.h"
#include "dopamine/archive/Storage.h"

namespace dopamine
{

/// @brief Main class, dispatching requests to generators and callbacks.
class Server
{
public:
    Server(
        mongo::DBClientConnection & connection,
        std::string const & database, std::string const & bulk_database,
        uint16_t port, authentication::AuthenticatorBase const & authenticator);

    ~Server();

    void run();

    // void shutdown();

private:
    mongo::DBClientConnection & _connection;
    std::string _database;
    std::string _bulk_database;
    uint16_t _port;
    authentication::AuthenticatorBase const & _authenticator;
    AccessControlList _acl;
    archive::Storage _storage;

    static odil::AssociationParameters _acceptor(
        odil::AssociationParameters const &,
        authentication::AuthenticatorBase const & authenticator);

    odil::SCPDispatcher _get_dispatcher(odil::Association & association) const;
};

} // namespace dopamine

#endif // _13a8d4a4_4144_4910_b54a_702ae291eac2
