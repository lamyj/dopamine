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

std::string const UNIQUE_SOP_INSTANCE_UID = "2.16.756.5.5.100.3611280983.20092.1364462458.1.0";
std::string const UNIQUE_STUDY_INSTANCE_UID = "2.16.756.5.5.100.3611280983.19057.1364461809.7789";
std::string const UNIQUE_SERIES_INSTANCE_UID = "2.16.756.5.5.100.3611280983.20092.1364462458.1";

struct TestDataWadoRS
{
    TestDataWadoRS()
    {
        std::string NetworkConfFILE(getenv("DOPAMINE_TEST_CONFIG"));
        dopamine::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);
    }

    ~TestDataWadoRS()
    {
        dopamine::ConfigurationPACS::delete_instance();
        sleep(1);
    }
};

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: wado_rs Accessors
 */
BOOST_FIXTURE_TEST_CASE(Accessors, TestDataWadoRS)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << UNIQUE_STUDY_INSTANCE_UID;

    dopamine::services::Wado_rs wadors(stream.str());

    BOOST_CHECK_NE(wadors.get_boundary(), "");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: wado_rs request (Study)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudy, TestDataWadoRS)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << UNIQUE_STUDY_INSTANCE_UID;

    // Create the response
    dopamine::services::Wado_rs wadors(stream.str());

    std::string test_filename(getenv("DOPAMINE_TEST_DICOMFILE"));
    test_filename = boost::filesystem::path(test_filename).filename().c_str();

    BOOST_CHECK_EQUAL(wadors.get_response() != "", true);
    BOOST_CHECK_EQUAL(wadors.get_response().size(), 1681);
    BOOST_CHECK_EQUAL(wadors.get_filename(), test_filename);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: wado_rs request (Study/Series)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudySeries, TestDataWadoRS)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << UNIQUE_STUDY_INSTANCE_UID << "/";
    stream << "series" << "/" << UNIQUE_SERIES_INSTANCE_UID;

    // Create the response
    dopamine::services::Wado_rs wadors(stream.str());

    std::string test_filename(getenv("DOPAMINE_TEST_DICOMFILE"));
    test_filename = boost::filesystem::path(test_filename).filename().c_str();

    BOOST_CHECK_EQUAL(wadors.get_response() != "", true);
    BOOST_CHECK_EQUAL(wadors.get_response().size(), 1681);
    BOOST_CHECK_EQUAL(wadors.get_filename(), test_filename);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: wado_rs request (Study/Series/Instance)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudySeriesInstance, TestDataWadoRS)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << UNIQUE_STUDY_INSTANCE_UID << "/";
    stream << "series" << "/" << UNIQUE_SERIES_INSTANCE_UID << "/";
    stream << "instances" << "/" << UNIQUE_SOP_INSTANCE_UID;

    // Create the response
    dopamine::services::Wado_rs wadors(stream.str());

    std::string test_filename(getenv("DOPAMINE_TEST_DICOMFILE"));
    test_filename = boost::filesystem::path(test_filename).filename().c_str();

    BOOST_CHECK_EQUAL(wadors.get_response() != "", true);
    BOOST_CHECK_EQUAL(wadors.get_response().size(), 1681);
    BOOST_CHECK_EQUAL(wadors.get_filename(), test_filename);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: No parameter
 */
BOOST_AUTO_TEST_CASE(MissingStudyParameter)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Unknown first parameter
 */
BOOST_AUTO_TEST_CASE(UnknownFirstParameter)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/unknown/value", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Missing Study Instance UID
 */
BOOST_AUTO_TEST_CASE(MissingStudyInstance)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/studies/", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/studies", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Unknown second parameter
 */
BOOST_AUTO_TEST_CASE(UnknownSecondParameter)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/studies/value/unknown/value", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Missing Series Instance UID
 */
BOOST_AUTO_TEST_CASE(MissingSeriesInstance)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/studies/value/series/", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/studies/value/series", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Unknown third parameter
 */
BOOST_AUTO_TEST_CASE(UnknownThirdParameter)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/studies/value/series/value/unknown/value", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Missing SOP Instance UID
 */
BOOST_AUTO_TEST_CASE(MissingSOPInstance)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/studies/value/series/value/instances/", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/studies/value/series/value/instances", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: dataset not find
 */
BOOST_FIXTURE_TEST_CASE(DatasetNotFind, TestDataWadoRS)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs("/studies/value/series/value/instances/value", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 404 && exc.statusmessage() == "Not Found"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: dataset location empty
 */
BOOST_FIXTURE_TEST_CASE(BadDatasetLocation, TestDataWadoRS)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << "2.16.756.5.5.100.3611280983.19057.1364461809.9999" << "/";
    stream << "series" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462499.1" << "/";
    stream << "instances" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462499.1.0";

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs(stream.str(), ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 404 && exc.statusmessage() == "Not Found"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Cannot not open file
 */
BOOST_FIXTURE_TEST_CASE(CannotOpenDataset, TestDataWadoRS)
{
    std::stringstream stream;
    stream << "/";
    stream << "studies" << "/" << "2.16.756.5.5.100.3611280983.19057.1364461809.8888" << "/";
    stream << "series" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462488.1" << "/";
    stream << "instances" << "/" << "2.16.756.5.5.100.3611280983.20092.1364462488.1.0";

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_rs(stream.str(), ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 500 && exc.statusmessage() == "Internal Server Error"); });
}
