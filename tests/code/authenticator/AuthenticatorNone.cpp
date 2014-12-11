/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleAuthenticatorNone
#include <boost/test/unit_test.hpp>

#include "authenticator/AuthenticatorNone.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    research_pacs::authenticator::AuthenticatorNone* authenticatorNone =
            new research_pacs::authenticator::AuthenticatorNone();

    BOOST_REQUIRE_EQUAL(authenticatorNone != NULL, true);

    delete authenticatorNone;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Execute with NULL identity (true)
 */
BOOST_AUTO_TEST_CASE(TEST_OK_02)
{
    research_pacs::authenticator::AuthenticatorNone* authenticatorNone =
            new research_pacs::authenticator::AuthenticatorNone();

    BOOST_REQUIRE_EQUAL((*authenticatorNone)(NULL), true);

    delete authenticatorNone;
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Execute with identity = NONE (true)
 */
BOOST_AUTO_TEST_CASE(TEST_OK_03)
{
    research_pacs::authenticator::AuthenticatorNone* authenticatorNone =
            new research_pacs::authenticator::AuthenticatorNone();

    UserIdentityNegotiationSubItemRQ * identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_NONE);

    BOOST_REQUIRE_EQUAL((*authenticatorNone)(identity), true);

    delete identity;
    delete authenticatorNone;
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: Execute with identity != NONE (false)
 */
BOOST_AUTO_TEST_CASE(TEST_OK_04)
{
    research_pacs::authenticator::AuthenticatorNone* authenticatorNone =
            new research_pacs::authenticator::AuthenticatorNone();

    // ASC_USER_IDENTITY_UNKNOWN
    UserIdentityNegotiationSubItemRQ * identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_UNKNOWN);
    BOOST_REQUIRE_EQUAL((*authenticatorNone)(identity), false);
    delete identity;

    // ASC_USER_IDENTITY_USER
    identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_USER);
    BOOST_REQUIRE_EQUAL((*authenticatorNone)(identity), false);
    delete identity;

    // ASC_USER_IDENTITY_USER_PASSWORD
    identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
    BOOST_REQUIRE_EQUAL((*authenticatorNone)(identity), false);
    delete identity;

    // ASC_USER_IDENTITY_KERBEROS
    identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_KERBEROS);
    BOOST_REQUIRE_EQUAL((*authenticatorNone)(identity), false);
    delete identity;

    // ASC_USER_IDENTITY_SAML
    identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_SAML);
    BOOST_REQUIRE_EQUAL((*authenticatorNone)(identity), false);
    delete identity;

    delete authenticatorNone;
}
