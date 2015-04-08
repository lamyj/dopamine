/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleWado_rs
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include <sstream>

#include "core/ConfigurationPACS.h"
#include "services/webservices/Wado_rs.h"
#include "services/webservices/WebServiceException.h"

/**
 * Pre-conditions:
 *     - we assume that ConfigurationPACS works correctly
 *
 *     - Following Environment variables should be defined
 *          * DOPAMINE_TEST_CONFIG
 *          * DOPAMINE_TEST_DICOMFILE
 */

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: wado_rs request (Study)
 */
struct TestDataOK01
{
    TestDataOK01()
    {
        std::string NetworkConfFILE(getenv("DOPAMINE_TEST_CONFIG"));
        dopamine::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);
    }

    ~TestDataOK01()
    {
        dopamine::ConfigurationPACS::delete_instance();
        sleep(1);
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_01, TestDataOK01)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << "2.16.756.5.5.100.3611280983.19057.1364461809.7789";

    // Create the response
    dopamine::services::Wado_rs wadors(stream.str());

    std::string test_filename(getenv("DOPAMINE_TEST_DICOMFILE"));
    test_filename = boost::filesystem::path(test_filename).filename().c_str();

    BOOST_CHECK_EQUAL(wadors.get_response() != "", true);
    BOOST_CHECK_EQUAL(wadors.get_response().size(), 1681);
    BOOST_CHECK_EQUAL(wadors.get_filename(), test_filename);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: wado_rs request (Study/Series)
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK01)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << "2.16.756.5.5.100.3611280983.19057.1364461809.7789" << "/";
    stream << "series" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462458.1";

    // Create the response
    dopamine::services::Wado_rs wadors(stream.str());

    std::string test_filename(getenv("DOPAMINE_TEST_DICOMFILE"));
    test_filename = boost::filesystem::path(test_filename).filename().c_str();

    BOOST_CHECK_EQUAL(wadors.get_response() != "", true);
    BOOST_CHECK_EQUAL(wadors.get_response().size(), 1681);
    BOOST_CHECK_EQUAL(wadors.get_filename(), test_filename);
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: wado_rs request (Study/Series/Instance)
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_03, TestDataOK01)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << "2.16.756.5.5.100.3611280983.19057.1364461809.7789" << "/";
    stream << "series" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462458.1" << "/";
    stream << "instances" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462458.1.0";

    // Create the response
    dopamine::services::Wado_rs wadors(stream.str());

    std::string test_filename(getenv("DOPAMINE_TEST_DICOMFILE"));
    test_filename = boost::filesystem::path(test_filename).filename().c_str();

    BOOST_CHECK_EQUAL(wadors.get_response() != "", true);
    BOOST_CHECK_EQUAL(wadors.get_response().size(), 1681);
    BOOST_CHECK_EQUAL(wadors.get_filename(), test_filename);
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: No parameter
 */
BOOST_AUTO_TEST_CASE(TEST_KO_01)
{
    std::string query = "/";

    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);

    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(""),
                        dopamine::services::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::services::Wado_rs wadors(query);
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}

/*************************** TEST KO 02 *******************************/
/**
 * Error test case: Unknown first parameter
 */
BOOST_AUTO_TEST_CASE(TEST_KO_02)
{
    std::string query = "/unknown/value";

    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::services::Wado_rs wadors(query);
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}

/*************************** TEST KO 03 *******************************/
/**
 * Error test case: Missing Study Instance UID
 */
BOOST_AUTO_TEST_CASE(TEST_KO_03)
{
    std::string query = "/studies/";

    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::services::Wado_rs wadors(query);
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);

    query = "/studies";
    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);
}

/*************************** TEST KO 04 *******************************/
/**
 * Error test case: Unknown second parameter
 */
BOOST_AUTO_TEST_CASE(TEST_KO_04)
{
    std::string query = "/studies/value/unknown/value";

    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::services::Wado_rs wadors(query);
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}

/*************************** TEST KO 05 *******************************/
/**
 * Error test case: Missing Series Instance UID
 */
BOOST_AUTO_TEST_CASE(TEST_KO_05)
{
    std::string query = "/studies/value/series/";

    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::services::Wado_rs wadors(query);
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);

    query = "/studies/value/series";
    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);
}

/*************************** TEST KO 06 *******************************/
/**
 * Error test case: Unknown third parameter
 */
BOOST_AUTO_TEST_CASE(TEST_KO_06)
{
    std::string query = "/studies/value/series/value/unknown/value";

    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::services::Wado_rs wadors(query);
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}

/*************************** TEST KO 07 *******************************/
/**
 * Error test case: Missing SOP Instance UID
 */
BOOST_AUTO_TEST_CASE(TEST_KO_07)
{
    std::string query = "/studies/value/series/value/instances/";

    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::services::Wado_rs wadors(query);
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);

    query = "/studies/value/series/value/instances";
    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);
}

/*************************** TEST KO 08 *******************************/
/**
 * Error test case: dataset not find
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_08, TestDataOK01)
{
    std::string query = "/studies/value/series/value/instances/value";

    // Create the response
    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        dopamine::services::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::services::Wado_rs wadors(query);
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 404);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Not Found");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}


/*************************** TEST KO 09 *******************************/
/**
 * Error test case: No database
 */
BOOST_AUTO_TEST_CASE(TEST_KO_09)
{
    std::string query = "/studies/value/series/value/instances/value";

    // Create the response
    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(query),
                        std::exception);
}

/*************************** TEST KO 10 *******************************/
/**
 * Error test case: dataset location empty
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_10, TestDataOK01)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << "2.16.756.5.5.100.3611280983.19057.1364461809.9999" << "/";
    stream << "series" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462499.1" << "/";
    stream << "instances" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462499.1.0";

    {
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(stream.str()),
                        dopamine::services::WebServiceException);
    }

    bool catch_exec = false;
    try
    {
        dopamine::services::Wado_rs wadors_(stream.str());
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 404);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Not Found");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}

/*************************** TEST KO 11 *******************************/
/**
 * Error test case: Cannot not open file
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_11, TestDataOK01)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << "2.16.756.5.5.100.3611280983.19057.1364461809.8888" << "/";
    stream << "series" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462488.1" << "/";
    stream << "instances" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462488.1.0";

    {
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::services::Wado_rs wadors(stream.str()),
                        dopamine::services::WebServiceException);
    }

    bool catch_exec = false;
    try
    {
        dopamine::services::Wado_rs wadors_(stream.str());
    }
    catch (dopamine::services::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 500);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Internal Server Error");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}
