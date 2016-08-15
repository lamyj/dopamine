/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE AuthenticatorCSV
#include <boost/test/unit_test.hpp>

#include <string>

#include <odil/AssociationParameters.h>

#include "dopamine/authentication/AuthenticatorCSV.h"
#include "dopamine/Exception.h"

#include "fixtures/CSV.h"

BOOST_FIXTURE_TEST_CASE(BadFilename, fixtures::CSV)
{
    BOOST_REQUIRE_THROW(
        dopamine::authentication::AuthenticatorCSV const authenticator(filename+"foo"),
        dopamine::Exception);
}

BOOST_FIXTURE_TEST_CASE(IdentityNone, fixtures::CSV)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_none();

    dopamine::authentication::AuthenticatorCSV const authenticator(filename);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsername, fixtures::CSV)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username("foo");

    dopamine::authentication::AuthenticatorCSV const authenticator(filename);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordOK, fixtures::CSV)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password("user2", "password2");

    dopamine::authentication::AuthenticatorCSV const authenticator(filename);
    BOOST_REQUIRE(authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordBadUsername, fixtures::CSV)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password("foo", "password2");

    dopamine::authentication::AuthenticatorCSV const authenticator(filename);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityUsernameAndPasswordBadPassword, fixtures::CSV)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password("user2", "password");

    dopamine::authentication::AuthenticatorCSV const authenticator(filename);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentityKerberos, fixtures::CSV)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_kerberos("foo");

    dopamine::authentication::AuthenticatorCSV const authenticator(filename);
    BOOST_REQUIRE(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(IdentitySAML, fixtures::CSV)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_saml("foo");

    dopamine::authentication::AuthenticatorCSV const authenticator(filename);
    BOOST_REQUIRE(!authenticator(parameters));
}
