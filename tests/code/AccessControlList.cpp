/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE AccessControlList
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/time.h>

#include <mongo/client/dbclient.h>

#include "dopamine/AccessControlList.h"

struct Fixture
{
    static unsigned long const seed;
    mongo::DBClientConnection connection;
    std::string database;

    static unsigned long get_seed()
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long const seed(1000000*tv.tv_sec + tv.tv_usec);

        std::srand(seed);
        return seed;
    }

    Fixture()
    {
        this->connection.connect("localhost");

        // Create a random database name
        for(int i=0; i<20; ++i)
        {
            this->database += 'A'+int(std::rand()/float(RAND_MAX)*26.);
        }

        this->connection.createCollection(this->database+".authorization");
    }

    ~Fixture()
    {
        // Drop database
        this->connection.dropDatabase(this->database);
    }
};

unsigned long const
Fixture
::seed(Fixture::get_seed());

BOOST_FIXTURE_TEST_CASE(Empty, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    BOOST_REQUIRE(acl.get_entries().empty());
}

BOOST_FIXTURE_TEST_CASE(Entries, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({
        { "principal1", "Echo", BSON("foo" << "bar") },
        { "principal2", "Query", BSON("plip" << "plop") } });
    BOOST_REQUIRE(!acl.get_entries().empty());

    auto entries = acl.get_entries();
    BOOST_REQUIRE_EQUAL(entries.size(), 2);

    std::sort(
        entries.begin(), entries.end(),
        [](dopamine::AccessControlList::Entry const & x, dopamine::AccessControlList::Entry const & y) {
            return x.principal<y.principal; });

    BOOST_REQUIRE(entries[0].principal == "principal1");
    BOOST_REQUIRE(entries[0].service == "Echo");
    BOOST_REQUIRE(entries[0].constraint == BSON("foo" << "bar"));

    BOOST_REQUIRE(entries[1].principal == "principal2");
    BOOST_REQUIRE(entries[1].service == "Query");
    BOOST_REQUIRE(entries[1].constraint == BSON("plip" << "plop"));
}

BOOST_FIXTURE_TEST_CASE(UnAuthenticatedWithNamed, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({{ "principal", "Echo", mongo::BSONObj() } });
    BOOST_REQUIRE(!acl.is_allowed("", "Echo"));
}

BOOST_FIXTURE_TEST_CASE(UnAuthenticatedWithWildcard, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({{ "*", "Echo", mongo::BSONObj() } });
    BOOST_REQUIRE(!acl.is_allowed("", "Echo"));
}

BOOST_FIXTURE_TEST_CASE(UnAuthenticated, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({{ "", "Echo", mongo::BSONObj() } });
    BOOST_REQUIRE(acl.is_allowed("", "Echo"));
}

BOOST_FIXTURE_TEST_CASE(UnAuthenticatedOtherService, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({{ "", "Store", mongo::BSONObj() } });
    BOOST_REQUIRE(!acl.is_allowed("", "Echo"));
}

BOOST_FIXTURE_TEST_CASE(AuthenticatedWithNamed, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({{ "principal", "Echo", mongo::BSONObj() } });
    BOOST_REQUIRE(acl.is_allowed("principal", "Echo"));
}

BOOST_FIXTURE_TEST_CASE(AuthenticatedWithWildcard, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({{ "*", "Echo", mongo::BSONObj() } });
    BOOST_REQUIRE(acl.is_allowed("principal", "Echo"));
}

BOOST_FIXTURE_TEST_CASE(Authenticated, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({{ "principal", "Echo", mongo::BSONObj() } });
    BOOST_REQUIRE(acl.is_allowed("principal", "Echo"));
}

BOOST_FIXTURE_TEST_CASE(AuthenticatedOtherPrincipal, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({{ "principal2", "Echo", mongo::BSONObj() } });
    BOOST_REQUIRE(!acl.is_allowed("principal1", "Echo"));
}

BOOST_FIXTURE_TEST_CASE(AuthenticatedOtherService, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({{ "principal", "Store", mongo::BSONObj() } });
    BOOST_REQUIRE(!acl.is_allowed("principal", "Echo"));
}

BOOST_FIXTURE_TEST_CASE(Constraints, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({
        { "principal", "Store", BSON("foo" << "bar") },
        { "principal", "Store", BSON("plip" << "plop") },
        { "principal", "Echo", mongo::BSONObj() },});

    auto const constraints = acl.get_constraints("principal", "Store");
    BOOST_REQUIRE(
        constraints == BSON("$or" << BSON_ARRAY(
            BSON("$and" << BSON_ARRAY(BSON("foo.Value" << "bar"))) <<
            BSON("$and" << BSON_ARRAY(BSON("plip.Value" << "plop")))
    )));
}

BOOST_FIXTURE_TEST_CASE(ConstraintsPassThrough, Fixture)
{
    dopamine::AccessControlList acl(this->connection, this->database);
    acl.set_entries({
        { "principal", "Store", BSON("foo" << "bar") },
        { "principal", "Echo", BSON("plip" << "plop") },
        { "principal", "Store", mongo::BSONObj() } });

    auto const constraints = acl.get_constraints("principal", "Store");
    BOOST_REQUIRE(constraints.isEmpty());
}
