/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <cstdlib>

#define BOOST_TEST_MODULE AuthenticatorLDAP
#include <boost/test/unit_test.hpp>

#include <odil/AssociationParameters.h>

#include "authenticator/AuthenticatorLDAP.h"
#include "core/ExceptionPACS.h"

/**
 * The following environment variables must be defined
 * * TEST_LDAP_SERVER
 * * TEST_LDAP_BIND
 * * TEST_LDAP_BASE
 * * TEST_LDAP_FILTER
 * * TEST_LDAP_USER
 * * TEST_LDAP_PASSWORD
 */

struct TestDataLDAP
{
    std::string server;
    std::string bind_user;
    std::string base;
    std::string filter;

    std::string user;
    std::string password;

    TestDataLDAP()
    {
        std::string server(getenv("TEST_LDAP_SERVER"));
        std::string bind_user(getenv("TEST_LDAP_BIND"));
        std::string base(getenv("TEST_LDAP_BASE"));
        std::string filter(getenv("TEST_LDAP_FILTER"));

        std::string user(getenv("TEST_LDAP_USER"));
        std::string password(getenv("TEST_LDAP_PASSWORD"));

        if(server.empty() || bind_user.empty() || base.empty() || filter.empty() ||
            user.empty() || password.empty())
        {
            throw dopamine::ExceptionPACS("Missing Environment Variables");
        }

        this->server = server;
        this->base = base;
        this->bind_user = bind_user;
        this->filter = filter;

        this->user = user;
        this->password = password;
    }
};

BOOST_FIXTURE_TEST_CASE(Constructor, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP const authenticator(
        server, bind_user, base, filter);
}

BOOST_FIXTURE_TEST_CASE(AuthorizationTrue, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP const authenticator(
        server, bind_user, base, filter);
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(user, password);
    BOOST_CHECK(authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(BadIdentityType, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP const authenticator(
        server, bind_user, base, filter);

    odil::AssociationParameters parameters;

    parameters.set_user_identity_to_none();
    BOOST_CHECK(!authenticator(parameters));

    parameters.set_user_identity_to_username("user");
    BOOST_CHECK(!authenticator(parameters));

    parameters.set_user_identity_to_kerberos("ticket");
    BOOST_CHECK(!authenticator(parameters));

    parameters.set_user_identity_to_saml("assertion");
    BOOST_CHECK(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(BadFilter, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP const authenticator(
        server, bind_user, base, "(uid=UnkownValue)");
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(user, password);
    BOOST_CHECK(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(InvalidFilter, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP const authenticator(
        server, bind_user, base, "(uid=");
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(user, password);
    BOOST_REQUIRE_THROW(authenticator(parameters), dopamine::ExceptionPACS);
}

BOOST_FIXTURE_TEST_CASE(BadServerAddress, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP const authenticator(
        "nowhere.example.com", bind_user, base, filter);
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(user, password);
    BOOST_REQUIRE_THROW(authenticator(parameters), dopamine::ExceptionPACS);
}

BOOST_FIXTURE_TEST_CASE(BadUser, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP const authenticator(
        server, bind_user, base, filter);
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(user+"a", password);
    BOOST_REQUIRE_THROW(authenticator(parameters), dopamine::ExceptionPACS);
}

BOOST_FIXTURE_TEST_CASE(BadPassword, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP const authenticator(
        server, bind_user, base, filter);
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(user, password+"a");
    BOOST_REQUIRE_THROW(authenticator(parameters), dopamine::ExceptionPACS);
}
