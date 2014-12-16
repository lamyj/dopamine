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

#include "authenticator/AuthenticatorLDAP.h"
#include "core/ExceptionPACS.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Get authorization => true
 */
struct TestDataOK01
{
    UserIdentityNegotiationSubItemRQ * identity = NULL;
    std::string ldap_server;
    std::string ldap_bind_user;
    std::string ldap_base;
    std::string ldap_filter;

    TestDataOK01()
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

        ldap_server = ldapserver;
        ldap_base = ldapbase;
        ldap_bind_user = bind;
        ldap_filter = "(cn=%user)";

        identity = new UserIdentityNegotiationSubItemRQ();
        identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
        identity->setPrimField(user.c_str(), user.length());
        identity->setSecField(password.c_str(), password.length());
    }

    ~TestDataOK01()
    {
        if (identity != NULL)
            delete identity;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_01, TestDataOK01)
{
    dopamine::authenticator::AuthenticatorLDAP* authenticatorldap =
            new dopamine::authenticator::AuthenticatorLDAP(ldap_server,
                                                                ldap_bind_user,
                                                                ldap_base,
                                                                ldap_filter);

    BOOST_CHECK_EQUAL((*authenticatorldap)(identity), true);

    delete authenticatorldap;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Get authorization => false (No Identity)
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK01)
{
    dopamine::authenticator::AuthenticatorLDAP* authenticatorldap =
            new dopamine::authenticator::AuthenticatorLDAP(ldap_server,
                                                                ldap_bind_user,
                                                                ldap_base,
                                                                ldap_filter);

    BOOST_CHECK_EQUAL((*authenticatorldap)(NULL), false);

    delete authenticatorldap;
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Get authorization => false (bad Identity type)
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_03, TestDataOK01)
{
    dopamine::authenticator::AuthenticatorLDAP* authenticatorldap =
            new dopamine::authenticator::AuthenticatorLDAP(ldap_server,
                                                                ldap_bind_user,
                                                                ldap_base,
                                                                ldap_filter);

    identity->setIdentityType(ASC_USER_IDENTITY_UNKNOWN);
    BOOST_CHECK_EQUAL((*authenticatorldap)(identity), false);

    delete authenticatorldap;
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: Get authorization => false (request failed)
 */
struct TestDataOK04
{
    UserIdentityNegotiationSubItemRQ * identity = NULL;
    std::string ldap_server;
    std::string ldap_bind_user;
    std::string ldap_base;
    std::string ldap_filter;

    TestDataOK04()
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

        ldap_server = ldapserver;
        ldap_base = ldapbase;
        ldap_bind_user = bind;
        ldap_filter = "(uid=UnkownValue)";

        identity = new UserIdentityNegotiationSubItemRQ();
        identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
        identity->setPrimField(user.c_str(), user.length());
        identity->setSecField(password.c_str(), password.length());
    }

    ~TestDataOK04()
    {
        if (identity != NULL)
            delete identity;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_04, TestDataOK04)
{
    dopamine::authenticator::AuthenticatorLDAP* authenticatorldap =
            new dopamine::authenticator::AuthenticatorLDAP(ldap_server,
                                                                ldap_bind_user,
                                                                ldap_base,
                                                                ldap_filter);

    BOOST_CHECK_EQUAL((*authenticatorldap)(identity), false);

    delete authenticatorldap;
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Authentication failed: Bad Server address
 */
struct TestDataKO01
{
    UserIdentityNegotiationSubItemRQ * identity = NULL;
    std::string ldap_server;
    std::string ldap_bind_user;
    std::string ldap_base;
    std::string ldap_filter;

   TestDataKO01()
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

       ldap_server = "bad_value";
       ldap_base = ldapbase;
       ldap_bind_user = bind;
       ldap_filter = "(cn=%user)";

       identity = new UserIdentityNegotiationSubItemRQ();
       identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
       identity->setPrimField(user.c_str(), user.length());
       identity->setSecField(password.c_str(), password.length());
   }

   ~TestDataKO01()
   {
       if (identity != NULL)
           delete identity;
   }
};

BOOST_FIXTURE_TEST_CASE(TEST_KO_01, TestDataKO01)
{
    dopamine::authenticator::AuthenticatorLDAP* authenticatorldap =
            new dopamine::authenticator::AuthenticatorLDAP(ldap_server,
                                                                ldap_bind_user,
                                                                ldap_base,
                                                                ldap_filter);

    BOOST_REQUIRE_THROW((*authenticatorldap)(identity),
                        dopamine::ExceptionPACS);

    delete authenticatorldap;
}

/*************************** TEST KO 02 *******************************/
/**
 * Error test case: Authentication failed: Bad Credential
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_02, TestDataOK01)
{
    dopamine::authenticator::AuthenticatorLDAP* authenticatorldap =
            new dopamine::authenticator::AuthenticatorLDAP(ldap_server,
                                                                ldap_bind_user,
                                                                ldap_base,
                                                                ldap_filter);

    identity->setPrimField("bad_user", 8);
    BOOST_REQUIRE_THROW((*authenticatorldap)(identity),
                        dopamine::ExceptionPACS);

    delete authenticatorldap;
}

/*************************** TEST KO 03 *******************************/
/**
 * Error test case: Authentication failed: Bad filter
 */
struct TestDataKO03
{
    UserIdentityNegotiationSubItemRQ * identity = NULL;
    std::string ldap_server;
    std::string ldap_bind_user;
    std::string ldap_base;
    std::string ldap_filter;

   TestDataKO03()
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

       ldap_server = ldapserver;
       ldap_base = ldapbase;
       ldap_bind_user = bind;
       ldap_filter = "(cbad=%user";

       identity = new UserIdentityNegotiationSubItemRQ();
       identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
       identity->setPrimField(user.c_str(), user.length());
       identity->setSecField(password.c_str(), password.length());
   }

   ~TestDataKO03()
   {
       if (identity != NULL)
           delete identity;
   }
};

BOOST_FIXTURE_TEST_CASE(TEST_KO_03, TestDataKO03)
{
    dopamine::authenticator::AuthenticatorLDAP* authenticatorldap =
            new dopamine::authenticator::AuthenticatorLDAP(ldap_server,
                                                                ldap_bind_user,
                                                                ldap_base,
                                                                ldap_filter);

    BOOST_REQUIRE_THROW((*authenticatorldap)(identity),
                        dopamine::ExceptionPACS);

    delete authenticatorldap;
}
