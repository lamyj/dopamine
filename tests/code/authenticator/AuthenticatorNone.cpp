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

#include "authenticator/AuthenticatorNone.h"

BOOST_AUTO_TEST_CASE(NONEIdentity)
{
    dopamine::authenticator::AuthenticatorNone const authenticator;

    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_none();

    BOOST_CHECK(authenticator(parameters));
}

BOOST_AUTO_TEST_CASE(BadIdentity)
{
    dopamine::authenticator::AuthenticatorNone const authenticator;

    odil::AssociationParameters parameters;

    parameters.set_user_identity_to_username("user");
    BOOST_CHECK(!authenticator(parameters));

    parameters.set_user_identity_to_username_and_password("user", "pwd");
    BOOST_CHECK(!authenticator(parameters));

    parameters.set_user_identity_to_kerberos("ticket");
    BOOST_CHECK(!authenticator(parameters));

    parameters.set_user_identity_to_saml("assertion");
    BOOST_CHECK(!authenticator(parameters));
}
