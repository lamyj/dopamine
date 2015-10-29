/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleAuthenticatorNone
#include <boost/test/unit_test.hpp>

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
 * Nominal test case: Execute with NULL identity (true)
 */
BOOST_AUTO_TEST_CASE(NULLIdentity)
{
    dopamine::authenticator::AuthenticatorNone authenticatorNone;

    BOOST_CHECK_EQUAL(authenticatorNone(NULL), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Execute with identity = NONE (true)
 */
BOOST_AUTO_TEST_CASE(NONEIdentity)
{
    dopamine::authenticator::AuthenticatorNone authenticatorNone;

    UserIdentityNegotiationSubItemRQ * identity =
            new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_NONE);

    BOOST_CHECK_EQUAL(authenticatorNone(identity), true);

    delete identity;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Execute with identity != NONE (false)
 */
BOOST_AUTO_TEST_CASE(BadIdentity)
{
    dopamine::authenticator::AuthenticatorNone authenticatorNone;

    // ASC_USER_IDENTITY_UNKNOWN
    UserIdentityNegotiationSubItemRQ * identity =
            new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_UNKNOWN);
    BOOST_CHECK_EQUAL(authenticatorNone(identity), false);
    delete identity;

    // ASC_USER_IDENTITY_USER
    identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_USER);
    BOOST_CHECK_EQUAL(authenticatorNone(identity), false);
    delete identity;

    // ASC_USER_IDENTITY_USER_PASSWORD
    identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
    BOOST_CHECK_EQUAL(authenticatorNone(identity), false);
    delete identity;

    // ASC_USER_IDENTITY_KERBEROS
    identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_KERBEROS);
    BOOST_CHECK_EQUAL(authenticatorNone(identity), false);
    delete identity;

    // ASC_USER_IDENTITY_SAML
    identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_SAML);
    BOOST_CHECK_EQUAL(authenticatorNone(identity), false);
    delete identity;
}
