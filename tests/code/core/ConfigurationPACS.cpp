/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#define BOOST_TEST_MODULE ModuleConfigurationPACS
#include <boost/test/unit_test.hpp>

#include "core/ConfigurationPACS.h"
#include "core/ExceptionPACS.h"

struct TestDataConfiguration
{
    std::string filename;

    TestDataConfiguration()
    {
        const char * conffile = getenv("DOPAMINE_TEST_CONFIG");
        if (conffile == NULL)
        {
            BOOST_FAIL("Missing environment variable: DOPAMINE_TEST_CONFIG");
        }
        else
        {
            filename = std::string(conffile);
        }
    }

    ~TestDataConfiguration()
    {
        dopamine::ConfigurationPACS::delete_instance();
    }
};

struct TestDataConfigurationBase
{
    std::string filename;

    TestDataConfigurationBase():
        filename("./tmp_test_moduleConfigurationPACS.ini")
    {
        // Nothing to do
    }

    virtual ~TestDataConfigurationBase()
    {
        remove(filename.c_str());
        dopamine::ConfigurationPACS::delete_instance();
    }
};

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::ConfigurationPACS& configuration =
            dopamine::ConfigurationPACS::get_instance();

    BOOST_CHECK(configuration.has_value("No_value") == false);

    dopamine::ConfigurationPACS::delete_instance();
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Parsing configuration file
 */
BOOST_FIXTURE_TEST_CASE(ParsingConfigurationFile, TestDataConfiguration)
{
    dopamine::ConfigurationPACS& confpacs =
            dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    BOOST_CHECK_EQUAL(confpacs.get_value("dicom.port"), "11112");
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Retrieve value
 */
BOOST_FIXTURE_TEST_CASE(GetValue, TestDataConfiguration)
{
    dopamine::ConfigurationPACS& confpacs =
            dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    BOOST_CHECK_EQUAL(confpacs.get_value("dicom.port"), "11112");
    BOOST_CHECK_EQUAL(confpacs.get_value("database", "port"), "27017");

    // Unknown key
    BOOST_CHECK_EQUAL(confpacs.get_value("not_know", "port"), "");
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Contains value
 */
BOOST_FIXTURE_TEST_CASE(HasValue, TestDataConfiguration)
{
    dopamine::ConfigurationPACS& confpacs =
            dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    BOOST_CHECK_EQUAL(confpacs.has_value("dicom.port"), true);
    BOOST_CHECK_EQUAL(confpacs.has_value("database", "port"), true);
    
    BOOST_CHECK_EQUAL(confpacs.has_value("badsection.port"), false);
    BOOST_CHECK_EQUAL(confpacs.has_value("database", "badfield"), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: testing get_database_configuration
 */
BOOST_FIXTURE_TEST_CASE(GetDatabaseConfig, TestDataConfiguration)
{
    dopamine::ConfigurationPACS& confpacs =
            dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);

    dopamine::MongoDBInformation db_info;
    std::string hostname = "";
    int port = -1;
    std::vector<std::string> indexes = {};
    confpacs.get_database_configuration(db_info, hostname, port, indexes);

    BOOST_CHECK_EQUAL(db_info.get_db_name(), "dopamine_test");
    BOOST_CHECK_EQUAL(db_info.get_bulk_data(), "pacs_bulk");
    BOOST_CHECK_EQUAL(hostname, "localhost");
    BOOST_CHECK_EQUAL(port, 27017);
    BOOST_CHECK_EQUAL(indexes.size(), 7);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Parsing failure => Unknown file
 */
BOOST_AUTO_TEST_CASE(BadFilename)
{
    dopamine::ConfigurationPACS& confpacs =
            dopamine::ConfigurationPACS::get_instance();
    BOOST_REQUIRE_THROW(confpacs.parse("badfilename"),
                        dopamine::ExceptionPACS);
    dopamine::ConfigurationPACS::delete_instance();
}
