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

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Logger Not Initialize
 *                    This test should be done in first.
 */
BOOST_AUTO_TEST_CASE(No_Initialization)
{
    // redirect standard output to stringstream
    std::stringstream stream;
    std::streambuf* OldBuf = std::cout.rdbuf(stream.rdbuf());

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    // set the default output
    std::cout.rdbuf(OldBuf);

    BOOST_CHECK_EQUAL(stream.str(), "");
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: InitializeLogger (ERROR)
 */
BOOST_AUTO_TEST_CASE(InitializeLogger_Error)
{
    // Initialize logger
    dopamine::initialize_logger("ERROR");

    // redirect standard output to stringstream
    std::stringstream stream;
    std::streambuf* OldBuf = std::cout.rdbuf(stream.rdbuf());

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    // set the default output
    std::cout.rdbuf(OldBuf);

    BOOST_CHECK_EQUAL(stream.str() != "", true);

    BOOST_CHECK_EQUAL(stream.str().find("ERROR") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("WARN") == std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("INFO") == std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("DEBUG") == std::string::npos, true);
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: InitializeLogger (WARNING)
 */
BOOST_AUTO_TEST_CASE(InitializeLogger_Warning)
{
    // Initialize logger
    dopamine::initialize_logger("WARNING");

    // redirect standard output to stringstream
    std::stringstream stream;
    std::streambuf* OldBuf = std::cout.rdbuf(stream.rdbuf());

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    // set the default output
    std::cout.rdbuf(OldBuf);

    BOOST_CHECK_EQUAL(stream.str() != "", true);

    BOOST_CHECK_EQUAL(stream.str().find("ERROR") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("WARN") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("INFO") == std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("DEBUG") == std::string::npos, true);
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: InitializeLogger (INFO)
 */
BOOST_AUTO_TEST_CASE(InitializeLogger_Info)
{
    // Initialize logger
    dopamine::initialize_logger("INFO");

    // redirect standard output to stringstream
    std::stringstream stream;
    std::streambuf* OldBuf = std::cout.rdbuf(stream.rdbuf());

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    // set the default output
    std::cout.rdbuf(OldBuf);

    BOOST_CHECK_EQUAL(stream.str() != "", true);

    BOOST_CHECK_EQUAL(stream.str().find("ERROR") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("WARN") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("INFO") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("DEBUG") == std::string::npos, true);
}

/*************************** TEST OK 05 *******************************/
/**
 * Nominal test case: InitializeLogger (DEBUG)
 */
BOOST_AUTO_TEST_CASE(InitializeLogger_Debug)
{
    // Initialize logger
    dopamine::initialize_logger("DEBUG");

    // redirect standard output to stringstream
    std::stringstream stream;
    std::streambuf* OldBuf = std::cout.rdbuf(stream.rdbuf());

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    // set the default output
    std::cout.rdbuf(OldBuf);

    BOOST_CHECK_EQUAL(stream.str() != "", true);

    BOOST_CHECK_EQUAL(stream.str().find("ERROR") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("WARN") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("INFO") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("DEBUG") != std::string::npos, true);
}

/*************************** TEST OK 06 *******************************/
/**
 * Nominal test case: InitializeLogger (DEFAULT)
 */
BOOST_AUTO_TEST_CASE(InitializeLogger_Default)
{
    // Initialize logger
    dopamine::initialize_logger("");

    // redirect standard output to stringstream
    std::stringstream stream;
    std::streambuf* OldBuf = std::cout.rdbuf(stream.rdbuf());

    // Test
    dopamine::logger_error() << " test ";
    dopamine::logger_warning() << " test ";
    dopamine::logger_info() << " test ";
    dopamine::logger_debug() << " test ";

    // set the default output
    std::cout.rdbuf(OldBuf);

    BOOST_CHECK_EQUAL(stream.str() != "", true);

    BOOST_CHECK_EQUAL(stream.str().find("ERROR") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("WARN") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("INFO") != std::string::npos, true);
    BOOST_CHECK_EQUAL(stream.str().find("DEBUG") != std::string::npos, true);
}
