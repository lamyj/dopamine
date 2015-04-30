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

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Study level)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudy_XML, TestDataRequest)
{
    std::string const query = "/studies";

    // PS3.18: 6.7.1.2.1.1 Study Matching - Table 6.7.1-1. QIDO-RS STUDY Search Query Keys
    std::vector<std::string> pathinfo_to_test =
    {
        "StudyDate=20130328",
        "StudyTime=101009",
//        "AccessionNumber=*",                  // Cannot be test => Null value
//        "ModalitiesInStudy=*",                // Cannot be test => Not present
        "ReferringPhysicianName=Greg^House",
        "PatientName=Doe^Jane",
        "PatientID=dopamine_test_01",
        "StudyInstanceUID=2.16.756.5.5.100.3611280983.19057.1364461809.7789",
        "StudyID=Study_id",

        // PS3.18: 6.7.1.1.1 {attributeID} encoding rules
        "00100010=Doe^J*",
        "0020000d=2.16.756.5.5.100.3611280983.19057.1364461809.7789",
        "0020000D=2.16.756.5.5.100.3611280983.19057.1364461809.7789",

        // Multi parameters
        "0020000d=2.16.756.5.5.100.3611280983.19057.1364461809.7789&PatientName=Doe^Jane&PatientID=dopamine_test_0?"
    };

    for (std::string const pathinfo : pathinfo_to_test)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(query,
                                               pathinfo,
                                               dopamine::services::MIME_TYPE_APPLICATION_DICOMXML,
                                               "");
        std::string const response_xml = qidors_xml.get_response();

        // Check response
        BOOST_CHECK_EQUAL(response_xml != "", true);
        std::stringstream boundary; boundary << "--" << qidors_xml.get_boundary();
        std::stringstream stream; stream << boundary.str() << "--";
        unsigned int count = 0;
        size_t position = response_xml.find(boundary.str());
        while (position != std::string::npos && position != response_xml.find(stream.str()))
        {
            ++count;
            position = response_xml.find(boundary.str(), position+1);
        }
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.3611280983.19057.1364461809.7789") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("<FamilyName>Doe</FamilyName>") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("<GivenName>Jane</GivenName>") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("STUDY") != std::string::npos, true);

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribut : dopamine::services::mandatory_study_attributes)
        {
            BOOST_CHECK_EQUAL(response_xml.find(attribut) != std::string::npos, true);
        }
    }
}

/*************************** TEST Nominal *******************************/
BOOST_FIXTURE_TEST_CASE(RequestStudy_JSON, TestDataRequest)
{
    std::stringstream querystream;
    querystream << "/studies";

    std::stringstream pathinfostream;
    pathinfostream << "StudyInstanceUID=2.16.756.5.5.100.3611280983.19057.1364461809.7789";

    // Perform query and get JSON response
    dopamine::services::Qido_rs qidors_json(querystream.str(),
                                            pathinfostream.str(),
                                            dopamine::services::MIME_TYPE_APPLICATION_JSON,
                                            "");
    std::string const response_json = qidors_json.get_response();

    // Check response
    BOOST_CHECK_EQUAL(response_json != "", true);
    std::string searchedtext = "00080052";
    unsigned int count = 0;
    size_t position = response_json.find(searchedtext);
    while (position != std::string::npos)
    {
        ++count;
        position = response_json.find(searchedtext, position+1);
    }
    BOOST_CHECK_EQUAL(count, 1);
    BOOST_CHECK_EQUAL(response_json.find("2.16.756.5.5.100.3611280983.19057.1364461809.7789") != std::string::npos, true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Study level)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudySeries_XML, TestDataRequest)
{
    std::string const query = "/studies/2.16.756.5.5.100.3611280983.19057.1364461809.7789/series";

    // PS3.18: 6.7.1.2.1.2 Series Matching - Table 6.7.1-1a. QIDO-RS SERIES Search Query Keys
    std::vector<std::string> pathinfo_to_test =
    {
        "Modality=MR",
        "SeriesInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1",
        "SeriesNumber=196609",
//        "PerformedProcedureStepStartDate=*",                      // Cannot be test => Not present
//        "PerformedProcedureStepStartTime=*",                      // Cannot be test => Not present
//        "RequestAttributeSequence.ScheduledProcedureStepID=*",    // Cannot be test => Not present
//        "RequestAttributeSequence.RequestedProcedureID=*",        // Cannot be test => Not present

        // PS3.18: 6.7.1.1.1 {attributeID} encoding rules
        "00080060=M?",
        "0020000e=2.16.756.5.5.100.3611280983.20092.1364462458.1",
        "0020000E=2.16.756.5.5.100.3611280983.20092.1364462458.1",

        // Multi parameters
        "0020000e=2.16.756.5.5.100.3611280983.20092.1364462458.1&Modality=MR&SeriesNumber=196609"
    };

    for (std::string const pathinfo : pathinfo_to_test)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(query,
                                               pathinfo,
                                               dopamine::services::MIME_TYPE_APPLICATION_DICOMXML,
                                               "");
        std::string const response_xml = qidors_xml.get_response();

        // Check response
        BOOST_CHECK_EQUAL(response_xml != "", true);
        std::stringstream boundary; boundary << "--" << qidors_xml.get_boundary();
        std::stringstream stream; stream << boundary.str() << "--";
        unsigned int count = 0;
        size_t position = response_xml.find(boundary.str());
        while (position != std::string::npos && position != response_xml.find(stream.str()))
        {
            ++count;
            position = response_xml.find(boundary.str(), position+1);
        }
        BOOST_CHECK_EQUAL(count, 1);
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.3611280983.19057.1364461809.7789") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.3611280983.20092.1364462458.1") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("SERIES") != std::string::npos, true);

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : dopamine::services::mandatory_series_attributes)
        {
            if (attribute != "0008103e")
            BOOST_CHECK_EQUAL(response_xml.find(attribute) != std::string::npos, true);
        }
    }
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

