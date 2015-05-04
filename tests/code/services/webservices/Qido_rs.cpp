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
#include "core/ExceptionPACS.h"
#include "services/webservices/Qido_rs.h"
#include "services/webservices/WebServiceException.h"

struct TestDataRequest
{
    // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
    std::vector<std::string> const mandatory_study_attributes =
    {
        "00080020", // Study Date
        "00080030", // Study Time
        "00080050", // Accession Number
        "00080056", // Instance Availability
        "00080061", // Modalities in Study
        "00080090", // Referring Physician's Name
        "00100010", // Patient's Name
        "00100020", // Patient ID
        "00100030", // Patient's Birth Date
        "00100040", // Patient's Sex
        "0020000d", // Study Instance UID
        "00200010", // Study ID
        "00201206", // Number of Study Related Series
        "00201208"  // Number of Study Related Instances
    };

    // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
    std::vector<std::string> const mandatory_series_attributes =
    {
        "00080060", // Modality
        "0020000e", // Series Instance UID
        "00200011", // Series Number
        "00201209"  // Number of Series Related Instances
    };

    // See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
    std::vector<std::string> const mandatory_instance_attributes =
    {
        "00080016", // SOP Class UID
        "00080018", // SOP Instance UID
        "00080056", // Instance Availability
        "00200013", // Instance Number
        "00280010", // Rows
        "00280011", // Columns
        "00280100"  // Bits Allocated
    };

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
 * Nominal test case: qido_rs Accessors
 */
BOOST_FIXTURE_TEST_CASE(Accessors, TestDataRequest)
{
    dopamine::services::Qido_rs qidors_xml("/studies",
                                           "StudyInstanceUID=no_data",
                                           dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

    BOOST_CHECK_EQUAL(qidors_xml.get_contenttype(), dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

    dopamine::services::Qido_rs qidors_json("/studies",
                                            "StudyInstanceUID=no_data",
                                            dopamine::services::MIME_TYPE_APPLICATION_JSON);

    BOOST_CHECK_EQUAL(qidors_json.get_contenttype(), dopamine::services::MIME_TYPE_APPLICATION_JSON);
}

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
        BOOST_CHECK_EQUAL(response_xml.find("STUDY") != std::string::npos, true);

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : mandatory_study_attributes)
        {
            if (response_xml.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
            }
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
 * Nominal test case: qido_rs request (Series level)
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

        // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
        for (std::string attribute : mandatory_series_attributes)
        {
            if (response_xml.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Series level)
 */
BOOST_FIXTURE_TEST_CASE(RequestSeries_XML, TestDataRequest)
{
    std::string const query = "/series";

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
        BOOST_CHECK_GE(count, 1);
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.3611280983.19057.1364461809.7789") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.3611280983.20092.1364462458.1") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("SERIES") != std::string::npos, true);

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : mandatory_study_attributes)
        {
            if (response_xml.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
            }
        }

        // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
        for (std::string attribute : mandatory_series_attributes)
        {
            if (response_xml.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Instance level)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudySeriesInstance_XML, TestDataRequest)
{
    std::string const query = "/studies/2.16.756.5.5.100.3611280983.19057.1364461809.7789/series/2.16.756.5.5.100.3611280983.20092.1364462458.1/instances";

    // PS3.18: 6.7.1.2.1.3 Instance Matching - Table 6.7.1-1b. QIDO-RS INSTANCE Search Query Keys
    std::vector<std::string> pathinfo_to_test =
    {
        "SOPClassUID=1.2.840.10008.5.1.4.1.1.4",
        "SOPInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1.0",
        "InstanceNumber=1",

        // PS3.18: 6.7.1.1.1 {attributeID} encoding rules
        "00080016=1.2.840.10008.5.1.4.1.1.4",
        "00200013=1",

        // Multi parameters
        "00080016=1.2.840.10008.5.1.4.1.1.4&SOPInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1.0&InstanceNumber=1"
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
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.3611280983.20092.1364462458.1.0") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("IMAGE") != std::string::npos, true);

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : mandatory_instance_attributes)
        {
            if (response_xml.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Instance level)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudyInstance_XML, TestDataRequest)
{
    std::string const query = "/studies/2.16.756.5.5.100.3611280983.19057.1364461809.7789/instances";

    // PS3.18: 6.7.1.2.1.3 Instance Matching - Table 6.7.1-1b. QIDO-RS INSTANCE Search Query Keys
    std::vector<std::string> pathinfo_to_test =
    {
        "SOPClassUID=1.2.840.10008.5.1.4.1.1.4",
        "SOPInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1.0",
        "InstanceNumber=1",

        // PS3.18: 6.7.1.1.1 {attributeID} encoding rules
        "00080016=1.2.840.10008.5.1.4.1.1.4",
        "00200013=1",

        // Multi parameters
        "00080016=1.2.840.10008.5.1.4.1.1.4&SOPInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1.0&InstanceNumber=1"
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
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.3611280983.20092.1364462458.1.0") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("IMAGE") != std::string::npos, true);

        // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
        for (std::string attribute : mandatory_series_attributes)
        {
            if (response_xml.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
            }
        }

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : mandatory_instance_attributes)
        {
            if (response_xml.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Instance level)
 */
BOOST_FIXTURE_TEST_CASE(RequestInstance_XML, TestDataRequest)
{
    std::string const query = "/instances";

    // PS3.18: 6.7.1.2.1.3 Instance Matching - Table 6.7.1-1b. QIDO-RS INSTANCE Search Query Keys
    std::vector<std::string> pathinfo_to_test =
    {
        "SOPClassUID=1.2.840.10008.5.1.4.1.1.4",
        "SOPInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1.0",
        "InstanceNumber=1",

        // PS3.18: 6.7.1.1.1 {attributeID} encoding rules
        "00080016=1.2.840.10008.5.1.4.1.1.4",
        "00200013=1",

        // Multi parameters
        "00080016=1.2.840.10008.5.1.4.1.1.4&SOPInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1.0&InstanceNumber=1"
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
        BOOST_CHECK_GE(count, 1);
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.3611280983.19057.1364461809.7789") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.3611280983.20092.1364462458.1") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.3611280983.20092.1364462458.1.0") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("IMAGE") != std::string::npos, true);

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : mandatory_study_attributes)
        {
            if (response_xml.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
            }
        }

        // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
        for (std::string attribute : mandatory_series_attributes)
        {
            if (response_xml.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
            }
        }

        // See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
        for (std::string attribute : mandatory_instance_attributes)
        {
            if (response_xml.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request IncludeField
 */
BOOST_FIXTURE_TEST_CASE(RequestIncludeField_XML, TestDataRequest)
{
    std::string const query = "/studies";

    // PS3.18: 6.7.1.2.1.1 Study Matching - Table 6.7.1-1. QIDO-RS STUDY Search Query Keys
    std::vector<std::string> field_to_test =
    {
        "00080012", // DA
        "00080013", // TM
        "00080070", // LO
        "00081010", // SH
        "00101030", // DS
        "00180021", // CS
        "00180022", // Null value
        "00180089"  // IS
    };

    std::stringstream pathinfo;
    pathinfo << "StudyInstanceUID=2.16.756.5.5.100.3611280983.19057.1364461809.7789&PatientName=Doe^Jane";

    for (std::string const field : field_to_test)
    {
        pathinfo << "&includefield=" << field;
    }

    // Perform query and get XML response
    dopamine::services::Qido_rs qidors_xml(query,
                                           pathinfo.str(),
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
    BOOST_CHECK_EQUAL(response_xml.find("STUDY") != std::string::npos, true);

    // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
    for (std::string attribute : mandatory_study_attributes)
    {
        if (response_xml.find(attribute) == std::string::npos)
        {
            std::stringstream streamerror;
            streamerror << "Missing mandatory attribute: " << attribute;
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
        }
    }

    for (std::string const field : field_to_test)
    {
        if (response_xml.find(field) == std::string::npos)
        {
            std::stringstream streamerror;
            streamerror << "Missing include field: " << field;
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request Limit
 */
BOOST_FIXTURE_TEST_CASE(RequestLimit_XML, TestDataRequest)
{
    std::string const query = "/studies";

    std::vector<unsigned int> limits = { 3, 2, 1, 4 };

    for (unsigned int limit : limits)
    {
        std::stringstream pathinfo;
        pathinfo << "StudyInstanceUID=2.16.756.5.5.100.1333920868.19866.1424334602.23";
        pathinfo << "&limit=" << limit;

        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(query,
                                               pathinfo.str(),
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
        if (limit > 3)
        {
            BOOST_CHECK_EQUAL(count, 3);
        }
        else
        {
            BOOST_CHECK_EQUAL(count, limit);
        }
        BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.1333920868.19866.1424334602.23") != std::string::npos, true);
        BOOST_CHECK_EQUAL(response_xml.find("STUDY") != std::string::npos, true);
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request Offset
 */
BOOST_FIXTURE_TEST_CASE(RequestOffset_XML, TestDataRequest)
{
    std::string const query = "/studies";

    std::vector<unsigned int> offsets = { 0, 1, 2, 3 };

    for (unsigned int offset : offsets)
    {
        std::stringstream pathinfo;
        pathinfo << "StudyInstanceUID=2.16.756.5.5.100.1333920868.19866.1424334602.23";
        pathinfo << "&offset=" << offset;

        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(query,
                                               pathinfo.str(),
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

        BOOST_CHECK_EQUAL(count, (3 - offset));

        if (offset < 3)
        {
            BOOST_CHECK_EQUAL(response_xml.find("2.16.756.5.5.100.1333920868.19866.1424334602.23") != std::string::npos, true);
            BOOST_CHECK_EQUAL(response_xml.find("STUDY") != std::string::npos, true);
        }
    }
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Bad query
 */
BOOST_FIXTURE_TEST_CASE(RequestBadQuery, TestDataRequest)
{
    std::vector<std::string> query_to_test =
    {
        "",
        "/",
        "BadValue",
        "/BadValue",
        "/studies/2.16.756.5.5.100.1333920868.19866.1424334602.23",
        "/studies/2.16.756.5.5.100.1333920868.19866.1424334602.23/BadValue",
        "/studies/2.16.756.5.5.100.1333920868.19866.1424334602.23/series/2.16.756.5.5.100.1333920868.31960.1424338206.1",
        "/studies/2.16.756.5.5.100.1333920868.19866.1424334602.23/series/2.16.756.5.5.100.1333920868.31960.1424338206.1/BadValue",
        "/studies/2.16.756.5.5.100.1333920868.19866.1424334602.23/series/2.16.756.5.5.100.1333920868.31960.1424338206.1/instances/BadValue",
        "/studies/2.16.756.5.5.100.1333920868.19866.1424334602.23/instances/BadValue",
        "/series/2.16.756.5.5.100.1333920868.31960.1424338206.1",
        "/series/2.16.756.5.5.100.1333920868.31960.1424338206.1/BadValue",
        "/instances/BadValue"
    };

    for (auto query : query_to_test)
    {
        try
        {
            dopamine::services::Qido_rs qidors_xml(query, "");

            std::stringstream streamerror;
            streamerror << "Error expected but No error detected";
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
        }
        catch (dopamine::services::WebServiceException const exc)
        {
            BOOST_CHECK_EQUAL(exc.status(), 400);
            BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
        }
    }
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Bad tag
 */
BOOST_FIXTURE_TEST_CASE(RequestBadTag, TestDataRequest)
{
    std::string const query = "/instances";

    std::stringstream pathinfo;
    pathinfo << "StudyInstanceUID=2.16.756.5.5.100.1333920868.19866.1424334602.23";
    pathinfo << "&NotADICOMTag=badvalue";

    try
    {
        dopamine::services::Qido_rs qidors_xml(query, pathinfo.str());

        std::stringstream streamerror;
        streamerror << "Error expected but No error detected";
        BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
    }
    catch (dopamine::services::WebServiceException const exc)
    {
        BOOST_CHECK_EQUAL(exc.status(), 400);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Bad Request");
    }
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Fuzzy Matching (not implemented)
 */
BOOST_FIXTURE_TEST_CASE(RequestFuzzyMatching, TestDataRequest)
{
    std::string const query = "/studies";

    std::stringstream pathinfo;
    pathinfo << "StudyInstanceUID=2.16.756.5.5.100.1333920868.19866.1424334602.23";
    pathinfo << "&fuzzymatching=true";

    try
    {
        dopamine::services::Qido_rs qidors_xml(query, pathinfo.str());

        std::stringstream streamerror;
        streamerror << "Error expected but No error detected";
        BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(streamerror.str()));
    }
    catch (dopamine::services::WebServiceException const exc)
    {
        BOOST_CHECK_EQUAL(exc.status(), 299);
        BOOST_CHECK_EQUAL(exc.statusmessage(), "Warning");
    }
}
