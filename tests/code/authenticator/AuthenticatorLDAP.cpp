/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <cstdlib>

#define BOOST_TEST_MODULE ModuleAuthenticatorLDAP
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/Association.h>

#include "authenticator/AuthenticatorLDAP.h"
#include "core/ExceptionPACS.h"

/**
 * Pre-conditions:
 *     - Following Environment variables should be defined
 *          * TEST_LDAP_SERVER
 *          * TEST_LDAP_BASE
 *          * TEST_LDAP_BIND
 *          * TEST_LDAP_USER
 *          * TEST_LDAP_PASSWORD
 */

struct TestDataLDAP
{
    dcmtkpp::Association association;
    std::string ldap_server;
    std::string ldap_bind_user;
    std::string ldap_base;
    std::string ldap_filter;

    TestDataLDAP()
    {
        std::string ldapserver(getenv("TEST_LDAP_SERVER"));
        std::string ldapbase(getenv("TEST_LDAP_BASE"));
        std::string bind(getenv("TEST_LDAP_BIND"));
        std::string user(getenv("TEST_LDAP_USER"));
        std::string password(getenv("TEST_LDAP_PASSWORD"));

        if (ldapserver == "" || ldapbase == "" || bind == "" ||
            user == "" || password == "")
        {
            throw dopamine::ExceptionPACS("Missing Environment Variables");
        }

        this->ldap_server     = ldapserver;
        this->ldap_base       = ldapbase;
        this->ldap_bind_user  = bind;
        this->ldap_filter     = "samaccountname=%user";

        association.set_user_identity_to_username_and_password(user, password);
    }

    ~TestDataLDAP()
    {
        // Nothing to do
    }
};

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Get authorization => true
 */
BOOST_FIXTURE_TEST_CASE(AuthorizationTrue, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP* authenticatorldap =
            new dopamine::authenticator::AuthenticatorLDAP(ldap_server,
                                                           ldap_bind_user,
                                                           ldap_base,
                                                           ldap_filter);

    BOOST_REQUIRE(authenticatorldap != NULL);

    BOOST_CHECK_EQUAL((*authenticatorldap)(association), true);

    delete authenticatorldap;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Get authorization => false (bad Identity type)
 */
BOOST_FIXTURE_TEST_CASE(BadIdentityType, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP authenticatorldap(ldap_server,
                                                                 ldap_bind_user,
                                                                 ldap_base,
                                                                 ldap_filter);

    association.set_user_identity_type(dcmtkpp::UserIdentityType::None);
    BOOST_CHECK_EQUAL(authenticatorldap(association), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Get authorization => false (request failed)
 */
BOOST_FIXTURE_TEST_CASE(AuthorizationFalse, TestDataLDAP)
{
    ldap_filter = "(uid=UnkownValue)";

    dopamine::authenticator::AuthenticatorLDAP authenticatorldap(ldap_server,
                                                                 ldap_bind_user,
                                                                 ldap_base,
                                                                 ldap_filter);

    BOOST_CHECK_EQUAL(authenticatorldap(association), false);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Authentication failed: Bad Server address
 */
BOOST_FIXTURE_TEST_CASE(BadServerAddress, TestDataLDAP)
{
    ldap_server = "bad_value";
    ldap_filter = "(cn=%user)";

    dopamine::authenticator::AuthenticatorLDAP authenticatorldap(ldap_server,
                                                                 ldap_bind_user,
                                                                 ldap_base,
                                                                 ldap_filter);

    BOOST_REQUIRE_THROW(authenticatorldap(association),
                        dopamine::ExceptionPACS);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Authentication failed: Bad Credential
 */
BOOST_FIXTURE_TEST_CASE(BadCredential, TestDataLDAP)
{
    dopamine::authenticator::AuthenticatorLDAP authenticatorldap(ldap_server,
                                                                 ldap_bind_user,
                                                                 ldap_base,
                                                                 ldap_filter);

    association.set_user_identity_primary_field("bad_user");
    BOOST_REQUIRE_THROW(authenticatorldap(association),
                        dopamine::ExceptionPACS);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Authentication failed: Bad filter
 */
BOOST_FIXTURE_TEST_CASE(BadFilter, TestDataLDAP)
{
    ldap_filter = "(cbad=%user";

    dopamine::authenticator::AuthenticatorLDAP authenticatorldap(ldap_server,
                                                                 ldap_bind_user,
                                                                 ldap_base,
                                                                 ldap_filter);

    BOOST_REQUIRE_THROW(authenticatorldap(association),
                        dopamine::ExceptionPACS);
}
