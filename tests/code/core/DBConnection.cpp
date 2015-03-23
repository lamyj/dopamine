/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleDBConnection
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/test/unit_test.hpp>

#include "core/ConfigurationPACS.h"
#include "core/DBConnection.h"
#include "core/ExceptionPACS.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor/Destructor
 *
{
    dopamine::DBConnection::get_instance();
    dopamine::DBConnection::delete_instance();
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Initialization
 *
struct TestDataOK02
{
    std::vector<std::string> indexlistvect;

    TestDataOK02()
    {
        std::string NetworkConfFILE(getenv("DOPAMINE_TEST_CONFIG"));
        dopamine::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);

        // Get all indexes
        std::string indexlist =
            dopamine::ConfigurationPACS::get_instance().GetValue("database.indexlist");
        boost::split(indexlistvect, indexlist, boost::is_any_of(";"));
    }

    ~TestDataOK02()
    {
        dopamine::DBConnection::delete_instance();
        dopamine::ConfigurationPACS::delete_instance();
    }
};

(Initialization, TestDataOK02)
{
    // Create and Initialize DB connection
    dopamine::DBConnection::get_instance().Initialize
        (
            dopamine::ConfigurationPACS::get_instance().GetValue("database.dbname"),
            dopamine::ConfigurationPACS::get_instance().GetValue("database.hostname"),
            dopamine::ConfigurationPACS::get_instance().GetValue("database.port"),
            indexlistvect
        );

    BOOST_CHECK_EQUAL(dopamine::DBConnection::get_instance().get_db_name(),
                      "dopamine_test");
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Connection
 *
(Connection, TestDataOK02)
{
    // Create and Initialize DB connection
    dopamine::DBConnection::get_instance().Initialize
        (
            dopamine::ConfigurationPACS::get_instance().GetValue("database.dbname"),
            dopamine::ConfigurationPACS::get_instance().GetValue("database.hostname"),
            dopamine::ConfigurationPACS::get_instance().GetValue("database.port"),
            indexlistvect
        );

    dopamine::DBConnection::get_instance().connect();
    // ok => no error
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: checkUserAuthorization
 *
(Check_Authorization, TestDataOK02)
{
    dopamine::DBConnection& instance = dopamine::DBConnection::get_instance();
    // Create and Initialize DB connection
    instance.Initialize
        (
            dopamine::ConfigurationPACS::get_instance().GetValue("database.dbname"),
            dopamine::ConfigurationPACS::get_instance().GetValue("database.hostname"),
            dopamine::ConfigurationPACS::get_instance().GetValue("database.port"),
            indexlistvect
        );

    instance.connect();

    UserIdentityNegotiationSubItemRQ * identity = new UserIdentityNegotiationSubItemRQ();
    identity->setIdentityType(ASC_USER_IDENTITY_USER_PASSWORD);
    identity->setPrimField("unknown", 7);
    identity->setSecField("password2", 9);

    BOOST_CHECK_EQUAL(instance.checkUserAuthorization(*identity, DIMSE_C_ECHO_RQ),
                      false);

    delete identity;
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Not initialize
 *
(Not_Initialize)
{
    dopamine::DBConnection::get_instance();

    BOOST_REQUIRE_THROW(dopamine::DBConnection::get_instance().connect(),
                        dopamine::ExceptionPACS);

    dopamine::DBConnection::delete_instance();
}

/*************************** TEST KO 02 *******************************/
/**
 * Error test case: Connection error
 *
(Connection_error, TestDataOK02)
{
    dopamine::DBConnection& instance = dopamine::DBConnection::get_instance();
    // Create and Initialize DB connection
    instance.Initialize
        (
            dopamine::ConfigurationPACS::get_instance().GetValue("database.dbname"),
            "BAD_VALUE",
            dopamine::ConfigurationPACS::get_instance().GetValue("database.port"),
            indexlistvect
        );

    BOOST_REQUIRE_THROW(instance.connect(), dopamine::ExceptionPACS);

    dopamine::DBConnection::delete_instance();
}*/
