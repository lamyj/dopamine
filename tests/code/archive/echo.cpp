/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE echo
#include <boost/test/unit_test.hpp>

#include <cstdlib>
#include <string>
#include <sys/time.h>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include <odil/message/CEchoRequest.h>
#include <odil/message/Response.h>
#include <odil/registry.h>

#include "dopamine/archive/echo.h"

struct Fixture
{
    static unsigned long const seed;
    mongo::DBClientConnection connection;
    std::string database;
    dopamine::AccessControlList acl;

    static unsigned long get_seed()
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long const seed(1000000*tv.tv_sec + tv.tv_usec);

        std::srand(seed);
        return seed;
    }

    static std::string get_random_name()
    {
        std::string name;
        // Create a random database name
        for(int i=0; i<20; ++i)
        {
            name += 'A'+int(std::rand()/float(RAND_MAX)*26.);
        }
        return name;
    }

    Fixture()
    : connection(), database(Fixture::get_random_name()),
       acl(this->connection, this->database)
    {
        this->connection.connect("localhost");
        acl.set_entries({
            { "echo_only", "Echo", mongo::BSONObj() },
            { "store_only", "Store", mongo::BSONObj() },
        });
    }

    ~Fixture()
    {
        // Drop database
        this->connection.dropDatabase(this->database);
    }
};

BOOST_FIXTURE_TEST_CASE(Echo, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username("echo_only");

    odil::message::CEchoRequest const request(
        1, odil::registry::VerificationSOPClass);

    auto const status = dopamine::archive::echo(
        this->connection, this->acl, parameters, request);
    BOOST_REQUIRE_EQUAL(status, odil::message::Response::Success);
}

BOOST_FIXTURE_TEST_CASE(EchoBadUser, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username("store_only");

    odil::message::CEchoRequest const request(
        1, odil::registry::VerificationSOPClass);

    auto const status = dopamine::archive::echo(
        this->connection, this->acl, parameters, request);
    BOOST_REQUIRE(odil::message::Response::is_failure(status));
}
