/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE AuthenticatorLDAP
#include <boost/test/unit_test.hpp>

#include <odil/AssociationParameters.h>

#include "dopamine/authentication/AuthenticatorLDAP.h"
#include "dopamine/Exception.h"

#include "fixtures/LDAP.h"

BOOST_FIXTURE_TEST_CASE(IdentityNone, fixtures::LDAP)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_none();

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsername, fixtures::LDAP)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username("foo");

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordOK, fixtures::LDAP)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(username, password);

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordBadUsername, fixtures::LDAP)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(
        username+"INVALID", password);

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordBadPassword, fixtures::LDAP)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(
        username, password+"INVALID");

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordBadURI, fixtures::LDAP)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(username, password);

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri+"INVALID", bind_dn_template);
    BOOST_REQUIRE_THROW(authenticator(parameters), dopamine::Exception);
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordBadTemplate, fixtures::LDAP)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(username, password);

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template+"INVALID");
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityKerberos, fixtures::LDAP)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_kerberos("foo");

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentitySAML, fixtures::LDAP)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_saml("foo");

    dopamine::authentication::AuthenticatorLDAP const authenticator(
        uri, bind_dn_template);
    BOOST_REQUIRE(!authenticator(parameters));
}
