/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE AuthenticatorCSV
#include <boost/test/unit_test.hpp>

#include <fstream>

#include <odil/AssociationParameters.h>

#include "authenticator/AuthenticatorCSV.h"
#include "core/ExceptionPACS.h"

struct TestDataCSV
{
    std::string filename;

    TestDataCSV():
        filename("./tmp_test_moduleAuthenticatorCSV.csv")
    {
        std::ofstream myfile;
        myfile.open(filename);
        myfile << "user1\tpassword1\n";
        myfile << "user2\tpassword2\n";
        myfile.close();
    }

    ~TestDataCSV()
    {
        remove(filename.c_str());
    }
};

BOOST_FIXTURE_TEST_CASE(Constructor, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV const authenticator(filename);
    BOOST_CHECK_EQUAL(authenticator.get_table_count(), 2);
}

BOOST_FIXTURE_TEST_CASE(AuthorizationTrue, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV const authenticator(filename);

    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password("user2", "password2");
    
    BOOST_CHECK(authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(NoIdentity, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV const authenticator(filename);

    odil::AssociationParameters parameters;

    BOOST_CHECK(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(BadUser, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV const authenticator(filename);

    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(
        "baduser", "password2");

    BOOST_CHECK(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(BadPassword, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV const authenticator(filename);

    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(
        "user2", "badpassword");

    BOOST_CHECK(!authenticator(parameters));
}

BOOST_FIXTURE_TEST_CASE(BadIdentityType, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV const authenticator(filename);

    odil::AssociationParameters parameters;

    parameters.set_user_identity_to_none();
    BOOST_CHECK(!authenticator(parameters));

    parameters.set_user_identity_to_username("user");
    BOOST_CHECK(!authenticator(parameters));

    parameters.set_user_identity_to_kerberos("ticket");
    BOOST_CHECK(!authenticator(parameters));

    parameters.set_user_identity_to_saml("assertion");
    BOOST_CHECK(!authenticator(parameters));
}

BOOST_AUTO_TEST_CASE(BadFilename)
{
    BOOST_REQUIRE_THROW(
        dopamine::authenticator::AuthenticatorCSV("badfilename"),
        dopamine::ExceptionPACS);
}
