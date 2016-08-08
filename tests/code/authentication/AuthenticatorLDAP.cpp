/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE AuthenticatorLDAP
#include <boost/test/unit_test.hpp>

#include <cstdlib>
#include <stdexcept>

#include <odil/AssociationParameters.h>

#include "dopamine/authentication/AuthenticatorLDAP.h"
#include "dopamine/Exception.h"

/**
 * The following environment variables must be defined
 * * URI
 * * BIND_DN_TEMPLATE
 * * USERNAME
 * * PASSWORD
 */

struct Fixture
{
    std::string uri;
    std::string bind_dn_template;
    std::string username;
    std::string password;

    Fixture()
    {
        this->uri = Fixture::get_environment_variable("URI");
        this->bind_dn_template = Fixture::get_environment_variable("BIND_DN_TEMPLATE");
        this->username = Fixture::get_environment_variable("USERNAME");
        this->password = Fixture::get_environment_variable("PASSWORD");
    }

    ~Fixture()
    {
        // Nothing to do
    }

    static std::string get_environment_variable(std::string const & name)
    {
        char const * const value = getenv(name.c_str());

        std::string result;
        if(value == nullptr)
        {
            throw std::runtime_error("Missing environment variable: "+name);
        }
        else
        {
            result = std::string(value);
        }

        return result;
    }
};

BOOST_FIXTURE_TEST_CASE(IdentityNone, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_none();

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsername, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username("foo");

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordOK, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(username, password);

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordBadUsername, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(
        username+"INVALID", password);

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordBadPassword, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(
        username, password+"INVALID");

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordBadURI, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(username, password);

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri+"INVALID", bind_dn_template);
    BOOST_REQUIRE_THROW(authenticator(parameters), dopamine::Exception);
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordBadTemplate, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(username, password);

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template+"INVALID");
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityKerberos, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_kerberos("foo");

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentitySAML, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_saml("foo");

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}
