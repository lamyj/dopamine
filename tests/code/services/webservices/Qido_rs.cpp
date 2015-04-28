/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleQido_rs
#include <boost/test/unit_test.hpp>

#include "core/ConfigurationPACS.h"
#include "services/webservices/Qido_rs.h"

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Study level)
 */
struct TestDataRequest
{
    TestDataRequest()
    {
        std::string NetworkConfFILE(getenv("DOPAMINE_TEST_CONFIG"));
        dopamine::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);
    }

    ~TestDataRequest()
    {
        dopamine::ConfigurationPACS::delete_instance();
        sleep(1);
    }
};

BOOST_FIXTURE_TEST_CASE(RequestStudy, TestDataRequest)
{
    std::stringstream querystream;
    querystream << "/studies";

    std::stringstream pathinfostream;
    pathinfostream << "StudyInstanceUID=2.16.756.5.5.100.3611280983.19057.1364461809.7789";

    // Create the response
    dopamine::services::Qido_rs qidors(querystream.str(), pathinfostream.str());

    std::string const response = qidors.get_response();

    BOOST_CHECK_EQUAL(response != "", true);
    BOOST_CHECK_EQUAL(response.find("2.16.756.5.5.100.3611280983.19057.1364461809.7789") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("<FamilyName>Doe</FamilyName>") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("<GivenName>Jane</GivenName>") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("00080052") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("STUDY") != std::string::npos, true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Series level)
 */
BOOST_FIXTURE_TEST_CASE(RequestSeries, TestDataRequest)
{
    std::stringstream querystream;
    querystream << "/studies/2.16.756.5.5.100.3611280983.19057.1364461809.7789/series";

    std::stringstream pathinfostream;
    pathinfostream << "SeriesInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1";

    // Create the response
    dopamine::services::Qido_rs qidors(querystream.str(), pathinfostream.str());

    std::string const response = qidors.get_response();

    BOOST_CHECK_EQUAL(response != "", true);
    BOOST_CHECK_EQUAL(response.find("2.16.756.5.5.100.3611280983.19057.1364461809.7789") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("2.16.756.5.5.100.3611280983.20092.1364462458.1") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("00080060") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("00080052") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("SERIES") != std::string::npos, true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Instance level)
 */
BOOST_FIXTURE_TEST_CASE(RequestInstance, TestDataRequest)
{
    std::stringstream querystream;
    querystream << "/studies/2.16.756.5.5.100.3611280983.19057.1364461809.7789"
                << "/series/2.16.756.5.5.100.3611280983.20092.1364462458.1/instances";

    std::stringstream pathinfostream;
    pathinfostream << "SOPInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1.0";

    // Create the response
    dopamine::services::Qido_rs qidors(querystream.str(), pathinfostream.str());

    std::string const response = qidors.get_response();

    BOOST_CHECK_EQUAL(response != "", true);
    BOOST_CHECK_EQUAL(response.find("2.16.756.5.5.100.3611280983.19057.1364461809.7789") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("2.16.756.5.5.100.3611280983.20092.1364462458.1") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("2.16.756.5.5.100.3611280983.20092.1364462458.1.0") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("00200013") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("00080052") != std::string::npos, true);
    BOOST_CHECK_EQUAL(response.find("IMAGE") != std::string::npos, true);
}

