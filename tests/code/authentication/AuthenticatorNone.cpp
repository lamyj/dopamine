/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE AuthenticatorNone
#include <boost/test/unit_test.hpp>

#include <odil/AssociationParameters.h>

#include "dopamine/authentication/AuthenticatorNone.h"

BOOST_AUTO_TEST_CASE(IdentityNone)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_none();

    dopamine::authentication::AuthenticatorNone const authenticator;
    BOOST_REQUIRE(authenticator(parameters));
}

BOOST_AUTO_TEST_CASE(IdentityUsername)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username("foo");

    dopamine::authentication::AuthenticatorNone const authenticator;
    BOOST_REQUIRE(authenticator(parameters));
}

BOOST_AUTO_TEST_CASE(IdentityUsernameAndPassword)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password("foo", "bar");

    dopamine::authentication::AuthenticatorNone const authenticator;
    BOOST_REQUIRE(authenticator(parameters));
}

BOOST_AUTO_TEST_CASE(IdentityKerberos)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_kerberos("foo");

    dopamine::authentication::AuthenticatorNone const authenticator;
    BOOST_REQUIRE(authenticator(parameters));
}

BOOST_AUTO_TEST_CASE(IdentitySAML)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_saml("foo");

    dopamine::authentication::AuthenticatorNone const authenticator;
    BOOST_REQUIRE(authenticator(parameters));
}
