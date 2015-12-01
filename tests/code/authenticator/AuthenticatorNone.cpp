/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleAuthenticatorNone
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/Association.h>

#include "authenticator/AuthenticatorNone.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::authenticator::AuthenticatorNone* authenticatorNone =
            new dopamine::authenticator::AuthenticatorNone();

    BOOST_CHECK(authenticatorNone != NULL);

    delete authenticatorNone;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Execute with identity = NONE (true)
 */
BOOST_AUTO_TEST_CASE(NONEIdentity)
{
    dopamine::authenticator::AuthenticatorNone authenticatorNone;

    dcmtkpp::Association association;
    association.set_user_identity_to_none();

    BOOST_CHECK_EQUAL(authenticatorNone(association), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Execute with identity != NONE (false)
 */
BOOST_AUTO_TEST_CASE(BadIdentity)
{
    dopamine::authenticator::AuthenticatorNone authenticatorNone;

    dcmtkpp::Association association;

    // ASC_USER_IDENTITY_USER
    association.set_user_identity_to_username("user");
    BOOST_CHECK_EQUAL(authenticatorNone(association), false);

    // ASC_USER_IDENTITY_USER_PASSWORD
    association.set_user_identity_to_username_and_password("user", "pwd");
    BOOST_CHECK_EQUAL(authenticatorNone(association), false);

    // ASC_USER_IDENTITY_KERBEROS
    association.set_user_identity_to_kerberos("ticket");
    BOOST_CHECK_EQUAL(authenticatorNone(association), false);

    // ASC_USER_IDENTITY_SAML
    association.set_user_identity_to_saml("assertion");
    BOOST_CHECK_EQUAL(authenticatorNone(association), false);
}
