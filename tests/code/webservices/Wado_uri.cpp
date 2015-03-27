/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleWado_uri
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include <sstream>

#include "core/ConfigurationPACS.h"
#include "webservices/Wado_uri.h"
#include "webservices/WebServiceException.h"

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
 * Nominal test case: wado_uri request
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
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_01, TestDataOK01)
{
    std::stringstream stream;
    stream << dopamine::webservices::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::webservices::STUDY_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.19057.1364461809.7789" << "&";
    stream << dopamine::webservices::SERIES_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.20092.1364462458.1" << "&";
    stream << dopamine::webservices::SOP_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.20092.1364462458.1.0";

    std::string filename = "";
    // Create the response
    std::string data = dopamine::webservices::wado_uri(stream.str(), filename);

    std::string test_filename(getenv("DOPAMINE_TEST_DICOMFILE"));
    test_filename = boost::filesystem::path(test_filename).filename().c_str();

    BOOST_CHECK_EQUAL(data != "", true);
    BOOST_CHECK_EQUAL(data.size(), 1524);
    BOOST_CHECK_EQUAL(filename, test_filename);
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Bad request Unknown parameter
 */
BOOST_AUTO_TEST_CASE(TEST_KO_01)
{
    std::string query = "unknown=value";

    std::string filename = "";
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::webservices::wado_uri(query, filename),
                        dopamine::webservices::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::webservices::wado_uri(query, filename);
    }
    catch (dopamine::webservices::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}

/*************************** TEST KO 02 *******************************/
/**
 * Error test case: Bad request Missing mandatory parameters
 */
BOOST_AUTO_TEST_CASE(TEST_KO_02)
{
    std::stringstream stream;
    stream << dopamine::webservices::REQUEST_TYPE << "=WADO";

    std::string filename = "";
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::webservices::wado_uri(stream.str(), filename),
                        dopamine::webservices::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::webservices::wado_uri(stream.str(), filename);
    }
    catch (dopamine::webservices::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);

    catch_exec = false;
    try
    {
        dopamine::webservices::wado_uri("", filename);
    }
    catch (dopamine::webservices::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}

/*************************** TEST KO 03 *******************************/
/**
 * Error test case: Not implemented
 */
BOOST_AUTO_TEST_CASE(TEST_KO_03)
{
    std::stringstream stream;
    stream << dopamine::webservices::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::webservices::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::webservices::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::webservices::SOP_INSTANCE_UID << "=" << "test" << "&";
    stream << "contentType" << "=" << "test";

    std::string filename = "";
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::webservices::wado_uri(stream.str(), filename),
                        dopamine::webservices::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::webservices::wado_uri(stream.str(), filename);
    }
    catch (dopamine::webservices::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 406);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Not Acceptable");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}

/*************************** TEST KO 04 *******************************/
/**
 * Error test case: bad request type
 */
BOOST_AUTO_TEST_CASE(TEST_KO_04)
{
    std::stringstream stream;
    stream << dopamine::webservices::REQUEST_TYPE << "=" << "BADVALUE" << "&";
    stream << dopamine::webservices::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::webservices::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::webservices::SOP_INSTANCE_UID << "=" << "test";

    std::string filename = "";
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::webservices::wado_uri(stream.str(), filename),
                        dopamine::webservices::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::webservices::wado_uri(stream.str(), filename);
    }
    catch (dopamine::webservices::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 406);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Not Acceptable");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}

/*************************** TEST KO 05 *******************************/
/**
 * Error test case: dataset not find
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_05, TestDataOK01)
{
    std::stringstream stream;
    stream << dopamine::webservices::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::webservices::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::webservices::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::webservices::SOP_INSTANCE_UID << "=" << "test";

    std::string filename = "";
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::webservices::wado_uri(stream.str(), filename),
                        dopamine::webservices::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::webservices::wado_uri(stream.str(), filename);
    }
    catch (dopamine::webservices::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 404);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Not Found");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}


/*************************** TEST KO 06 *******************************/
/**
 * Error test case: No database
 */
BOOST_AUTO_TEST_CASE(TEST_KO_06)
{
    std::stringstream stream;
    stream << dopamine::webservices::REQUEST_TYPE << "=" << "BADVALUE" << "&";
    stream << dopamine::webservices::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::webservices::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::webservices::SOP_INSTANCE_UID << "=" << "test";

    std::string filename = "";
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::webservices::wado_uri(stream.str(), filename),
                        std::exception);
}

/*************************** TEST KO 07 *******************************/
/**
 * Error test case: dataset location empty
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_07, TestDataOK01)
{
    std::stringstream stream;
    stream << dopamine::webservices::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::webservices::STUDY_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.19057.1364461809.9999" << "&";
    stream << dopamine::webservices::SERIES_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.20092.1364462499.1" << "&";
    stream << dopamine::webservices::SOP_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.20092.1364462499.1.0";

    std::string filename = "";
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::webservices::wado_uri(stream.str(), filename),
                        dopamine::webservices::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::webservices::wado_uri(stream.str(), filename);
    }
    catch (dopamine::webservices::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 404);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Not Found");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}

/*************************** TEST KO 08 *******************************/
/**
 * Error test case: Cannot not open file
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_08, TestDataOK01)
{
    std::stringstream stream;
    stream << dopamine::webservices::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::webservices::STUDY_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.19057.1364461809.8888" << "&";
    stream << dopamine::webservices::SERIES_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.20092.1364462488.1" << "&";
    stream << dopamine::webservices::SOP_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.20092.1364462488.1.0";

    std::string filename = "";
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::webservices::wado_uri(stream.str(), filename),
                        dopamine::webservices::WebServiceException);

    bool catch_exec = false;
    try
    {
        dopamine::webservices::wado_uri(stream.str(), filename);
    }
    catch (dopamine::webservices::WebServiceException &exc)
    {
        catch_exec = true;

        BOOST_CHECK_EQUAL(exc.status(), 500);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Internal Server Error");
    }

    BOOST_CHECK_EQUAL(catch_exec, true);
}
