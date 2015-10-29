/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleLoggerPACS
#include <boost/test/unit_test.hpp>

#include "core/LoggerPACS.h"

struct TestDataLogger
{
    std::stringstream stream;
    std::streambuf* OldBuf;

    TestDataLogger()
    {
        // redirect standard output to stringstream
        OldBuf = std::cout.rdbuf(stream.rdbuf());
    }

    ~TestDataLogger()
    {
        // set the default output
        std::cout.rdbuf(OldBuf);
    }
};

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Logger Not Initialize
 *                    This test should be done in first.
 */
BOOST_FIXTURE_TEST_CASE(No_Initialization, TestDataLogger)
{
    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    BOOST_CHECK_EQUAL(stream.str(), "");
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: InitializeLogger (ERROR)
 */
BOOST_FIXTURE_TEST_CASE(InitializeLogger_Error, TestDataLogger)
{
    // Initialize logger
    dopamine::initialize_logger("ERROR");

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    BOOST_REQUIRE(stream.str() != "");

    BOOST_CHECK(stream.str().find("ERROR") != std::string::npos);
    BOOST_CHECK(stream.str().find("WARN") == std::string::npos);
    BOOST_CHECK(stream.str().find("INFO") == std::string::npos);
    BOOST_CHECK(stream.str().find("DEBUG") == std::string::npos);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: InitializeLogger (WARNING)
 */
BOOST_FIXTURE_TEST_CASE(InitializeLogger_Warning, TestDataLogger)
{
    // Initialize logger
    dopamine::initialize_logger("WARNING");

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    BOOST_REQUIRE(stream.str() != "");

    BOOST_CHECK(stream.str().find("ERROR") != std::string::npos);
    BOOST_CHECK(stream.str().find("WARN") != std::string::npos);
    BOOST_CHECK(stream.str().find("INFO") == std::string::npos);
    BOOST_CHECK(stream.str().find("DEBUG") == std::string::npos);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: InitializeLogger (INFO)
 */
BOOST_FIXTURE_TEST_CASE(InitializeLogger_Info, TestDataLogger)
{
    // Initialize logger
    dopamine::initialize_logger("INFO");

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    BOOST_REQUIRE(stream.str() != "");

    BOOST_CHECK(stream.str().find("ERROR") != std::string::npos);
    BOOST_CHECK(stream.str().find("WARN") != std::string::npos);
    BOOST_CHECK(stream.str().find("INFO") != std::string::npos);
    BOOST_CHECK(stream.str().find("DEBUG") == std::string::npos);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: InitializeLogger (DEBUG)
 */
BOOST_FIXTURE_TEST_CASE(InitializeLogger_Debug, TestDataLogger)
{
    // Initialize logger
    dopamine::initialize_logger("DEBUG");

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    BOOST_REQUIRE(stream.str() != "");

    BOOST_CHECK(stream.str().find("ERROR") != std::string::npos);
    BOOST_CHECK(stream.str().find("WARN") != std::string::npos);
    BOOST_CHECK(stream.str().find("INFO") != std::string::npos);
    BOOST_CHECK(stream.str().find("DEBUG") != std::string::npos);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: InitializeLogger (DEFAULT)
 */
BOOST_FIXTURE_TEST_CASE(InitializeLogger_Default, TestDataLogger)
{
    // Initialize logger
    dopamine::initialize_logger("");

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    BOOST_REQUIRE(stream.str() != "");

    BOOST_CHECK(stream.str().find("ERROR") != std::string::npos);
    BOOST_CHECK(stream.str().find("WARN") != std::string::npos);
    BOOST_CHECK(stream.str().find("INFO") != std::string::npos);
    BOOST_CHECK(stream.str().find("DEBUG") != std::string::npos);
}
