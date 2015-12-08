/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleWado_uri

#include <sstream>

#include <boost/filesystem.hpp>

//#include "services/webservices/Wado_uri.h"
//#include "services/webservices/WebServiceException.h"
#include "../ServicesTestClass.h"

/**
 * Pre-conditions:
 *     - we assume that ConfigurationPACS works correctly
 *
 *     - Following Environment variables should be defined
 *          * DOPAMINE_TEST_CONFIG
 *          * DOPAMINE_TEST_DICOMFILE
 */

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: wado_rs Accessors
 *
BOOST_FIXTURE_TEST_CASE(Accessors, ServicesTestClass)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "="
           << STUDY_INSTANCE_UID_01_01 << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "="
           << SERIES_INSTANCE_UID_01_01_01 << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "="
           << SOP_INSTANCE_UID_01_01_01_01;

    dopamine::services::Wado_uri wadouri(stream.str());

    BOOST_CHECK_EQUAL(wadouri.get_boundary(), "");
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: wado_uri request
 *
BOOST_FIXTURE_TEST_CASE(RequestStudySeriesInstance, ServicesTestClass)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "="
           << STUDY_INSTANCE_UID_01_01 << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "="
           << SERIES_INSTANCE_UID_01_01_01 << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "="
           << SOP_INSTANCE_UID_01_01_01_01;

    // Create the response
    dopamine::services::Wado_uri wadouri(stream.str(), "");

    std::string data = wadouri.get_response();

    BOOST_REQUIRE(data != "");

    std::stringstream stream_dataset; stream_dataset << data;
    auto file = dcmtkpp::Reader::read_file(stream_dataset);
    auto const dataset = file.second;

    BOOST_CHECK_EQUAL(dataset.as_string(dcmtkpp::registry::SOPInstanceUID)[0],
                      SOP_INSTANCE_UID_01_01_01_01);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Bad request Unknown parameter
 *
BOOST_AUTO_TEST_CASE(UnknownParameters)
{
    BOOST_CHECK_EXCEPTION(
                dopamine::services::Wado_uri("unknown=value", ""),
                dopamine::services::WebServiceException,
                [] (dopamine::services::WebServiceException const exc)
                    { return (exc.status() == 400 &&
                              exc.statusmessage() == "Bad Request"); });
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Bad request Missing mandatory parameters
 *
BOOST_AUTO_TEST_CASE(MissingMandatoryParameters)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=WADO";

    BOOST_CHECK_EXCEPTION(
                dopamine::services::Wado_uri(stream.str(), ""),
                dopamine::services::WebServiceException,
                [] (dopamine::services::WebServiceException const exc)
                    { return (exc.status() == 400 &&
                              exc.statusmessage() == "Bad Request"); });

    BOOST_CHECK_EXCEPTION(
                dopamine::services::Wado_uri("", ""),
                dopamine::services::WebServiceException,
                [] (dopamine::services::WebServiceException const exc)
                    { return (exc.status() == 400 &&
                              exc.statusmessage() == "Bad Request"); });
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Not implemented
 *
BOOST_AUTO_TEST_CASE(NotImplemented)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << "test" << "&";
    stream << "contentType" << "=" << "test";

    BOOST_CHECK_EXCEPTION(
                dopamine::services::Wado_uri(stream.str(), ""),
                dopamine::services::WebServiceException,
                [] (dopamine::services::WebServiceException const exc)
                    { return (exc.status() == 406 &&
                              exc.statusmessage() == "Not Acceptable"); });
}

/******************************* TEST Error ************************************/
/**
 * Error test case: bad request type
 *
BOOST_AUTO_TEST_CASE(BadRequestType)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "BADVALUE" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << "test";

    BOOST_CHECK_EXCEPTION(
                dopamine::services::Wado_uri(stream.str(), ""),
                dopamine::services::WebServiceException,
                [] (dopamine::services::WebServiceException const exc)
                    { return (exc.status() == 406 &&
                              exc.statusmessage() == "Not Acceptable"); });
}

/******************************* TEST Error ************************************/
/**
 * Error test case: dataset not find
 *
BOOST_FIXTURE_TEST_CASE(DatasetNotFind, ServicesTestClass)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << "test";

    BOOST_CHECK_EXCEPTION(
                dopamine::services::Wado_uri(stream.str(), ""),
                dopamine::services::WebServiceException,
                [] (dopamine::services::WebServiceException const exc)
                    { return (exc.status() == 404 &&
                              exc.statusmessage() == "Not Found"); });
}

/******************************* TEST Error ************************************/
/**
 * Error test case: No database
 *
BOOST_AUTO_TEST_CASE(DatabaseNotConnected)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "=" << "test" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "=" << "test";

    BOOST_CHECK_EXCEPTION(
            dopamine::services::Wado_uri(stream.str(), ""),
            dopamine::services::WebServiceException,
            [] (dopamine::services::WebServiceException const exc)
                { return (exc.status() == 500 &&
                          exc.statusmessage() == "Internal Server Error"); });
}

/******************************* TEST Error ************************************/
/**
 * Error test case: dataset cannot be return
 *
BOOST_FIXTURE_TEST_CASE(BadDatasetBufferValue, ServicesTestClass)
{
    std::stringstream stream;
    stream << dopamine::services::REQUEST_TYPE << "=" << "WADO" << "&";
    stream << dopamine::services::STUDY_INSTANCE_UID << "="
           << "2.16.756.5.5.100.3611280983.20092.123456789" << "&";
    stream << dopamine::services::SERIES_INSTANCE_UID << "="
           << "2.16.756.5.5.100.3611280983.20092.123456789.0" << "&";
    stream << dopamine::services::SOP_INSTANCE_UID << "="
           << "2.16.756.5.5.100.3611280983.20092.123456789.0.0";

    BOOST_CHECK_EXCEPTION(
            dopamine::services::Wado_uri(stream.str(), ""),
            dopamine::services::WebServiceException,
            [] (dopamine::services::WebServiceException const exc)
                { return (exc.status() == 500 &&
                          exc.statusmessage() == "Internal Server Error"); });
}
*/
