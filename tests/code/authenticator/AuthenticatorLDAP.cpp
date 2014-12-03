/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <cstdlib>

#define BOOST_TEST_MODULE ModuleAuthenticatorLDAP
#include <boost/test/unit_test.hpp>

#include "authenticator/AuthenticatorLDAP.h"
#include "core/ConfigurationPACS.h"
#include "core/ExceptionPACS.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Get authorization => true
 */
struct TestDataOK01
{
    std::string filename;
    UserIdentityNegotiationSubItemRQ * identity = NULL;

    bool couldbepassed;

    TestDataOK01()
    {
        filename = "./tmp_test_moduleAuthenticatorLDAP.ini";

        std::string ldapserver(getenv("TEST_LDAP_SERVER"));
        std::string ldapbase(getenv("TEST_LDAP_BASE"));
        std::string bind(getenv("TEST_LDAP_BIND"));
        std::string user(getenv("TEST_LDAP_USER"));
        std::string password(getenv("TEST_LDAP_PASSWORD"));

        couldbepassed = (ldapserver != "" && ldapbase != "" && bind != "" && user != "" && password != "");

        std::ofstream myfile;
        myfile.open(filename);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "allowed_peers=*\n";
        myfile << "port=11112\n";
        myfile << "[authenticator]\n";
        myfile << "type=LDAP\n";
        myfile << "ldap_server=" << ldapserver << "\n";
        myfile << "ldap_base=" << ldapbase << "\n";
        myfile << "ldap_bind_user=" << bind << "\n";
        myfile << "ldap_filter=(cn=%user)\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=\n";
        myfile.close();

        research_pacs::ConfigurationPACS::get_instance().Parse(filename);

        identity = new UserIdentityNegotiationSubItemRQ();
        identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
        identity->setPrimField(user.c_str(), user.length());
        identity->setSecField(password.c_str(), password.length());
    }

    ~TestDataOK01()
    {
        remove(filename.c_str());

        research_pacs::ConfigurationPACS::delete_instance();

        if (identity != NULL)
            delete identity;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_01, TestDataOK01)
{
    if (couldbepassed)
    {
        authenticator::AuthenticatorLDAP* authenticatorldap =
                new authenticator::AuthenticatorLDAP();

        BOOST_CHECK_EQUAL((*authenticatorldap)(identity), true);

        delete authenticatorldap;
    }
    else
    {
        throw research_pacs::ExceptionPACS("Missing Environment Variables");
    }
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Get authorization => false (No Identity)
 */
BOOST_AUTO_TEST_CASE(TEST_OK_02)
{
    authenticator::AuthenticatorLDAP* authenticatorldap = new authenticator::AuthenticatorLDAP();

    BOOST_CHECK_EQUAL((*authenticatorldap)(NULL), false);

    delete authenticatorldap;
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Get authorization => false (bad Identity type)
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_03, TestDataOK01)
{
    authenticator::AuthenticatorLDAP* authenticatorldap = new authenticator::AuthenticatorLDAP();

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
    std::string filename;
    UserIdentityNegotiationSubItemRQ * identity = NULL;

    bool couldbepassed;

    TestDataOK04()
    {
        filename = "./tmp_test_moduleAuthenticatorLDAP.ini";

        std::string ldapserver(getenv("TEST_LDAP_SERVER"));
        std::string ldapbase(getenv("TEST_LDAP_BASE"));
        std::string bind(getenv("TEST_LDAP_BIND"));
        std::string user(getenv("TEST_LDAP_USER"));
        std::string password(getenv("TEST_LDAP_PASSWORD"));

        couldbepassed = (ldapserver != "" && ldapbase != "" && bind != "" && user != "" && password != "");

        std::ofstream myfile;
        myfile.open(filename);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "allowed_peers=*\n";
        myfile << "port=11112\n";
        myfile << "[authenticator]\n";
        myfile << "type=LDAP\n";
        myfile << "ldap_server=" << ldapserver << "\n";
        myfile << "ldap_base=" << ldapbase << "\n";
        myfile << "ldap_bind_user=" << bind << "\n";
        myfile << "ldap_filter=(uid=UnkownValue)\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=\n";
        myfile.close();

        research_pacs::ConfigurationPACS::get_instance().Parse(filename);

        identity = new UserIdentityNegotiationSubItemRQ();
        identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
        identity->setPrimField(user.c_str(), user.length());
        identity->setSecField(password.c_str(), password.length());
    }

    ~TestDataOK04()
    {
        remove(filename.c_str());

        research_pacs::ConfigurationPACS::delete_instance();

        if (identity != NULL)
            delete identity;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_04, TestDataOK04)
{
    if (couldbepassed)
    {
        authenticator::AuthenticatorLDAP* authenticatorldap =
                new authenticator::AuthenticatorLDAP();

        BOOST_CHECK_EQUAL((*authenticatorldap)(identity), false);

        delete authenticatorldap;
    }
    else
    {
        throw research_pacs::ExceptionPACS("Missing Environment Variables");
    }
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Authentication failed: Bad Server address
 */
struct TestDataKO01
{
   std::string filename;
   UserIdentityNegotiationSubItemRQ * identity = NULL;

   bool couldbepassed;

   TestDataKO01()
   {
       filename = "./tmp_test_moduleAuthenticatorLDAP.ini";

       std::string ldapserver(getenv("TEST_LDAP_SERVER"));
       std::string ldapbase(getenv("TEST_LDAP_BASE"));
       std::string bind(getenv("TEST_LDAP_BIND"));
       std::string user(getenv("TEST_LDAP_USER"));
       std::string password(getenv("TEST_LDAP_PASSWORD"));

       couldbepassed = (ldapserver != "" && ldapbase != "" && bind != "" && user != "" && password != "");

       std::ofstream myfile;
       myfile.open(filename);
       myfile << "[dicom]\n";
       myfile << "storage_path=./temp_dir\n";
       myfile << "allowed_peers=*\n";
       myfile << "port=11112\n";
       myfile << "[authenticator]\n";
       myfile << "type=LDAP\n";
       myfile << "ldap_server=bad_value\n";
       myfile << "ldap_base=" << ldapbase << "\n";
       myfile << "ldap_bind_user=" << bind << "\n";
       myfile << "ldap_filter=(cn=%user)\n";
       myfile << "[listAddressPort]\n";
       myfile << "allowed=\n";
       myfile.close();

       research_pacs::ConfigurationPACS::get_instance().Parse(filename);

       identity = new UserIdentityNegotiationSubItemRQ();
       identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
       identity->setPrimField(user.c_str(), user.length());
       identity->setSecField(password.c_str(), password.length());
   }

   ~TestDataKO01()
   {
       remove(filename.c_str());

       research_pacs::ConfigurationPACS::delete_instance();

       if (identity != NULL)
           delete identity;
   }
};

BOOST_FIXTURE_TEST_CASE(TEST_KO_01, TestDataKO01)
{
    authenticator::AuthenticatorLDAP* authenticatorldap = new authenticator::AuthenticatorLDAP();

    BOOST_REQUIRE_THROW((*authenticatorldap)(identity),
                        research_pacs::ExceptionPACS);

    delete authenticatorldap;
}

/*************************** TEST KO 02 *******************************/
/**
 * Error test case: Authentication failed: Bad Credential
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_02, TestDataOK01)
{
    authenticator::AuthenticatorLDAP* authenticatorldap = new authenticator::AuthenticatorLDAP();

    identity->setPrimField("bad_user", 8);
    BOOST_REQUIRE_THROW((*authenticatorldap)(identity),
                        research_pacs::ExceptionPACS);

    delete authenticatorldap;
}

/*************************** TEST KO 03 *******************************/
/**
 * Error test case: Authentication failed: Bad filter
 */
struct TestDataKO03
{
   std::string filename;
   UserIdentityNegotiationSubItemRQ * identity = NULL;

   bool couldbepassed;

   TestDataKO03()
   {
       filename = "./tmp_test_moduleAuthenticatorLDAP.ini";

       std::string ldapserver(getenv("TEST_LDAP_SERVER"));
       std::string ldapbase(getenv("TEST_LDAP_BASE"));
       std::string bind(getenv("TEST_LDAP_BIND"));
       std::string user(getenv("TEST_LDAP_USER"));
       std::string password(getenv("TEST_LDAP_PASSWORD"));

       couldbepassed = (ldapserver != "" && ldapbase != "" && bind != "" && user != "" && password != "");

       std::ofstream myfile;
       myfile.open(filename);
       myfile << "[dicom]\n";
       myfile << "storage_path=./temp_dir\n";
       myfile << "allowed_peers=*\n";
       myfile << "port=11112\n";
       myfile << "[authenticator]\n";
       myfile << "type=LDAP\n";
       myfile << "ldap_server=" << ldapserver << "\n";
       myfile << "ldap_base=" << ldapbase << "\n";
       myfile << "ldap_bind_user=" << bind << "\n";
       myfile << "ldap_filter=(cbad=%user\n";
       myfile << "[listAddressPort]\n";
       myfile << "allowed=\n";
       myfile.close();

       research_pacs::ConfigurationPACS::get_instance().Parse(filename);

       identity = new UserIdentityNegotiationSubItemRQ();
       identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
       identity->setPrimField(user.c_str(), user.length());
       identity->setSecField(password.c_str(), password.length());
   }

   ~TestDataKO03()
   {
       remove(filename.c_str());

       research_pacs::ConfigurationPACS::delete_instance();

       if (identity != NULL)
           delete identity;
   }
};

BOOST_FIXTURE_TEST_CASE(TEST_KO_03, TestDataKO03)
{
    authenticator::AuthenticatorLDAP* authenticatorldap = new authenticator::AuthenticatorLDAP();

    BOOST_REQUIRE_THROW((*authenticatorldap)(identity),
                        research_pacs::ExceptionPACS);

    delete authenticatorldap;
}
