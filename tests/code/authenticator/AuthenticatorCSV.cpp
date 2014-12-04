/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleAuthenticatorCSV
#include <boost/test/unit_test.hpp>

#include "authenticator/AuthenticatorCSV.h"
#include "core/ExceptionPACS.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
 struct TestDataOK01
{
    std::string filename;
 
    TestDataOK01()
    {
        filename = "./tmp_test_moduleAuthenticatorCSV.csv";
        
        std::ofstream myfile;
        myfile.open(filename);
        myfile << "user1\tpassword1\n";
        myfile << "user2\tpassword2\n";
        myfile.close();
    }
 
    ~TestDataOK01()
    {
        remove(filename.c_str());
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_01, TestDataOK01)
{
    research_pacs::authenticator::AuthenticatorCSV* authenticatorcsv =
            new research_pacs::authenticator::AuthenticatorCSV(filename);
    
    BOOST_CHECK_EQUAL(authenticatorcsv->get_table_count(), 2);
    
    delete authenticatorcsv;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Get authorization => true
 */
 struct TestDataOK02
{
    std::string filename;
    research_pacs::authenticator::AuthenticatorCSV* authenticatorcsv;
 
    TestDataOK02()
    {
        filename = "./tmp_test_moduleAuthenticatorCSV.csv";
        
        std::ofstream myfile;
        myfile.open(filename);
        myfile << "user1\tpassword1\n";
        myfile << "user2\tpassword2\n";
        myfile.close();
        
        authenticatorcsv = new research_pacs::authenticator::AuthenticatorCSV(filename);
    }
 
    ~TestDataOK02()
    {
        remove(filename.c_str());
        delete authenticatorcsv;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK02)
{
    UserIdentityNegotiationSubItemRQ * identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
    identity->setPrimField("user2", 5);
    identity->setSecField("password2", 9);
    
    BOOST_CHECK_EQUAL((*authenticatorcsv)(identity), true);
    
    delete identity;
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Empty identity => false
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_03, TestDataOK02)
{
    BOOST_CHECK_EQUAL((*authenticatorcsv)(NULL), false);
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: Request with Bad user => false
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_04, TestDataOK02)
{
    UserIdentityNegotiationSubItemRQ * identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
    identity->setPrimField("baduser", 5);
    identity->setSecField("password2", 9);
    
    BOOST_CHECK_EQUAL((*authenticatorcsv)(identity), false);
    
    delete identity;
}

/*************************** TEST OK 05 *******************************/
/**
 * Nominal test case: Request with Bad password => false
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_05, TestDataOK02)
{
    UserIdentityNegotiationSubItemRQ * identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
    identity->setPrimField("user2", 5);
    identity->setSecField("badpassword", 9);
    
    BOOST_CHECK_EQUAL((*authenticatorcsv)(identity), false);
    
    delete identity;
}

/*************************** TEST OK 06 *******************************/
/**
 * Nominal test case: Request with Bad identity type => false
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_06, TestDataOK02)
{
    UserIdentityNegotiationSubItemRQ * identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_KERBEROS);
    identity->setPrimField("user2", 5);
    identity->setSecField("password2", 9);
    
    BOOST_CHECK_EQUAL((*authenticatorcsv)(identity), false);
    
    delete identity;
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Construction failure => Unknown file
 */
BOOST_AUTO_TEST_CASE(TEST_KO_01)
{
    BOOST_REQUIRE_THROW(new research_pacs::authenticator::AuthenticatorCSV("badfilename"),
                        research_pacs::ExceptionPACS);
}
