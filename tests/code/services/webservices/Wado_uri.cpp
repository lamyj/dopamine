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
#include "services/webservices/Wado_uri.h"
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

struct TestDataWadoURI
{
    TestDataWadoURI()
    {
        std::string NetworkConfFILE(getenv("DOPAMINE_TEST_CONFIG"));
        dopamine::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);
    }

    ~TestDataWadoURI()
    {
        dopamine::ConfigurationPACS::delete_instance();
    }
};

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: wado_rs Accessors
 */
BOOST_FIXTURE_TEST_CASE(Accessors, TestDataWadoURI)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << UNIQUE_STUDY_INSTANCE_UID << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << UNIQUE_SERIES_INSTANCE_UID << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << UNIQUE_SOP_INSTANCE_UID;

    dopamine::services::Wado_uri wadouri(stream.str());

    BOOST_CHECK_EQUAL(wadouri.get_boundary(), "");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: wado_uri request
 */

BOOST_FIXTURE_TEST_CASE(RequestStudySeriesInstance, TestDataWadoURI)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << UNIQUE_STUDY_INSTANCE_UID << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << UNIQUE_SERIES_INSTANCE_UID << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << UNIQUE_SOP_INSTANCE_UID;

    // Create the response
    dopamine::services::Wado_uri wadouri(stream.str(), "");

    std::string test_filename(getenv("DOPAMINE_TEST_DICOMFILE"));
    test_filename = boost::filesystem::path(test_filename).filename().c_str();

    std::string data = wadouri.get_response();
    std::string const filename = wadouri.get_filename();

    BOOST_CHECK_EQUAL(data != "", true);
    BOOST_CHECK_EQUAL(data.size(), 1524);
    BOOST_CHECK_EQUAL(filename, test_filename);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Bad request Unknown parameter
 */
BOOST_AUTO_TEST_CASE(UnknownParameters)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_uri("unknown=value", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Bad request Missing mandatory parameters
 */
BOOST_AUTO_TEST_CASE(MissingMandatoryParameters)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=WADO";

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_uri(stream.str(), ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_uri("", ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 400 && exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Not implemented
 */
BOOST_AUTO_TEST_CASE(NotImplemented)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << "test" << "&";
    stream << "contentType" << "=" << "test";

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_uri(stream.str(), ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 406 && exc.statusmessage() == "Not Acceptable"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: bad request type
 */
BOOST_AUTO_TEST_CASE(BadRequestType)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "BADVALUE" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << "test";

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_uri(stream.str(), ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 406 && exc.statusmessage() == "Not Acceptable"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: dataset not find
 */
BOOST_FIXTURE_TEST_CASE(DatasetNotFind, TestDataWadoURI)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << "test";

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_uri(stream.str(), ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 404 && exc.statusmessage() == "Not Found"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: No database
 */
BOOST_AUTO_TEST_CASE(DatabaseNotConnected)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "BADVALUE" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << "test";

    std::string filename = "";
    // Create the response
    BOOST_REQUIRE_THROW(dopamine::services::Wado_uri(stream.str(), ""),
                        std::exception);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: dataset location empty
 */
BOOST_FIXTURE_TEST_CASE(BadDatasetLocation, TestDataWadoURI)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.19057.1364461809.9999" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.20092.1364462499.1" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.20092.1364462499.1.0";

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_uri(stream.str(), ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 404 && exc.statusmessage() == "Not Found"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Cannot not open file
 */
BOOST_FIXTURE_TEST_CASE(CannotOpenDataset, TestDataWadoURI)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.19057.1364461809.8888" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.20092.1364462488.1" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << "2.16.756.5.5.100.3611280983.20092.1364462488.1.0";

    BOOST_CHECK_EXCEPTION(dopamine::services::Wado_uri(stream.str(), ""),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
        { return (exc.status() == 500 && exc.statusmessage() == "Internal Server Error"); });
}
