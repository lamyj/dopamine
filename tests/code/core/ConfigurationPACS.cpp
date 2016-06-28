/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#define BOOST_TEST_MODULE ConfigurationPACS
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
};

BOOST_AUTO_TEST_CASE(Constructor)
{
    auto const & configuration = dopamine::ConfigurationPACS::get_instance();
    BOOST_CHECK(!configuration.has_value("No_value"));
}

BOOST_FIXTURE_TEST_CASE(ParsingConfigurationFile, TestDataConfiguration)
{
    auto & configuration = dopamine::ConfigurationPACS::get_instance();
    configuration.parse(filename);
    BOOST_CHECK_EQUAL(configuration.get_value("dicom.port"), "11112");
}

BOOST_FIXTURE_TEST_CASE(GetValue, TestDataConfiguration)
{
    auto & configuration = dopamine::ConfigurationPACS::get_instance();
    configuration.parse(filename);
    
    BOOST_CHECK_EQUAL(configuration.get_value("dicom.port"), "11112");
    BOOST_CHECK_EQUAL(configuration.get_value("database", "port"), "27017");

    // Unknown key
    BOOST_CHECK_EQUAL(configuration.get_value("not_know", "port"), "");
}

BOOST_FIXTURE_TEST_CASE(HasValue, TestDataConfiguration)
{
    auto & configuration = dopamine::ConfigurationPACS::get_instance();
    configuration.parse(filename);
    
    BOOST_CHECK(configuration.has_value("dicom.port"));
    BOOST_CHECK(configuration.has_value("database", "port"));
    
    BOOST_CHECK(!configuration.has_value("badsection.port"));
    BOOST_CHECK(!configuration.has_value("database", "badfield"));
}

BOOST_FIXTURE_TEST_CASE(GetDatabaseConfig, TestDataConfiguration)
{
    auto & configuration = dopamine::ConfigurationPACS::get_instance();
    configuration.parse(filename);

    dopamine::MongoDBInformation db_info;
    std::string hostname = "";
    int port = -1;
    std::vector<std::string> indexes = {};
    configuration.get_database_configuration(db_info, hostname, port, indexes);

    BOOST_CHECK_EQUAL(db_info.get_db_name(), "dopamine_test");
    BOOST_CHECK_EQUAL(db_info.get_bulk_data(), "pacs_bulk");
    BOOST_CHECK_EQUAL(hostname, "localhost");
    BOOST_CHECK_EQUAL(port, 27017);
    BOOST_CHECK(indexes == std::vector<std::string>({
        "SOPInstanceUID", "PatientName", "PatientID",
        "SeriesInstanceUID", "SeriesDescription", "StudyInstanceUID",
        "StudyDescription"}));
}

BOOST_AUTO_TEST_CASE(BadFilename)
{
    auto & configuration = dopamine::ConfigurationPACS::get_instance();
    BOOST_REQUIRE_THROW(
        configuration.parse("badfilename"), dopamine::ExceptionPACS);
}
