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

struct TestDataSpecificAllowedAETitle : public TestDataConfigurationBase
{
    TestDataSpecificAllowedAETitle() : TestDataConfigurationBase()
    {
        std::ofstream myfile;
        myfile.open(filename);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "allowed_peers=USER1,USER3\n";
        myfile << "port=11112\n";
        myfile << "[database]\n";
        myfile << "hostname=localhost\n";
        myfile << "port=27017\n";
        myfile << "dbname=pacs\n";
        myfile << "[authenticator]\n";
        myfile << "type=CSV\n";
        myfile << "filepath=./authentest.csv\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=LANGUEDOC,LOCAL\n";
        myfile << "LANGUEDOC=languedoc:11113\n";
        myfile << "LOCAL=vexin:11112\n";
        myfile.close();
    }

    virtual ~TestDataSpecificAllowedAETitle()
    {
        // Nothing to do
    }
};

struct TestDataMissingField : public TestDataConfigurationBase
{
    TestDataMissingField() : TestDataConfigurationBase()
    {
        std::ofstream myfile;
        myfile.open(filename);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "port=11112\n";
        myfile << "[database]\n";
        myfile << "hostname=localhost\n";
        myfile << "port=27017\n";
        myfile << "dbname=pacs\n";
        myfile << "[authenticator]\n";
        myfile << "type=CSV\n";
        myfile << "filepath=./authentest.csv\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=LANGUEDOC,LOCAL\n";
        myfile << "LANGUEDOC=languedoc:11113\n";
        myfile << "LOCAL=vexin:11112\n";
        myfile.close();
    }

    virtual ~TestDataMissingField()
    {
        // Nothing to do
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
 * Nominal test case: Accessors
 */
BOOST_FIXTURE_TEST_CASE(Accessors, TestDataConfiguration)
{
    dopamine::ConfigurationPACS& confpacs =
            dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);

    auto aet = confpacs.get_aetitles();
    BOOST_REQUIRE_EQUAL(aet.size(), 1);
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
 * Nominal test case: testing peerInAETitle Everybody Allowed ('*')
 */
BOOST_FIXTURE_TEST_CASE(AllowedAETtitle, TestDataConfiguration)
{
    dopamine::ConfigurationPACS& confpacs =
            dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("VALUE"), true);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("LOCAL"), true);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("NOERROR"), true);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle(""), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: testing peerInAETitle Specific user Allowed
 */

BOOST_FIXTURE_TEST_CASE(SpecificAllowedAETitle, TestDataSpecificAllowedAETitle)
{
    dopamine::ConfigurationPACS& confpacs =
            dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("USER1"), true);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("USER3"), true);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("USER2"), false);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle(""), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: testing peerForAETitle
 */
BOOST_FIXTURE_TEST_CASE(GetAddressForAETitle, TestDataConfiguration)
{
    dopamine::ConfigurationPACS& confpacs =
            dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    std::string address;
    BOOST_CHECK_EQUAL(confpacs.peer_for_aetitle("LOCAL", address), true);
    BOOST_CHECK_EQUAL(address, "localhost:11113");
    
    address = "value";
    BOOST_CHECK_EQUAL(confpacs.peer_for_aetitle("ERROR", address), false);
    BOOST_CHECK_EQUAL(address, "");
    
    address = "value";
    BOOST_CHECK_EQUAL(confpacs.peer_for_aetitle("", address), false);
    BOOST_CHECK_EQUAL(address, "");
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

    std::string db_name = "";
    std::string bulk_db = "";
    std::string hostname = "";
    int port = -1;
    std::vector<std::string> indexes = {};
    confpacs.get_database_configuration(db_name, bulk_db, hostname, port, indexes);

    BOOST_CHECK_EQUAL(db_name, "dopamine_test");
    BOOST_CHECK_EQUAL(bulk_db, "dopamine_test");
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

/******************************* TEST Error ************************************/
/**
 * Error test case: Parsing failure
 *                  => Missing mandatory field dicom.allowed_peers
 */
BOOST_FIXTURE_TEST_CASE(MissingMandatoryField, TestDataMissingField)
{
    dopamine::ConfigurationPACS& confpacs =
            dopamine::ConfigurationPACS::get_instance();
    BOOST_REQUIRE_THROW(confpacs.parse(filename),
                        dopamine::ExceptionPACS);
}
