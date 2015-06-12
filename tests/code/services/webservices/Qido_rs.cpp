/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleQido_rs

#include <mimetic/mimeentity.h>

#include "core/ExceptionPACS.h"
#include "services/webservices/Qido_rs.h"
#include "services/webservices/WebServiceException.h"
#include "../ServicesTestClass.h"

class TestDataRequest : public ServicesTestClass
{
public:
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

    TestDataRequest() : ServicesTestClass()
    {
        // Nothing to do
    }

    virtual ~TestDataRequest()
    {
        // Nothing to do
    }
};

mimetic::MimeEntity to_MIME_message(std::string const & message,
                                    std::string const & boundary)
{
    BOOST_REQUIRE(message != "");

    // Parse MIME Message
    std::stringstream streamresponse;
    streamresponse << dopamine::services::MIME_VERSION << "\n"
                   << dopamine::services::CONTENT_TYPE
                   << dopamine::services::MIME_TYPE_MULTIPART_RELATED << "; "
                   << dopamine::services::ATTRIBUT_BOUNDARY
                   << boundary << "\n" << "\n";
    streamresponse << message << "\n";
    return mimetic::MimeEntity(streamresponse);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs Accessors
 */
BOOST_FIXTURE_TEST_CASE(Accessors, TestDataRequest)
{
    dopamine::services::Qido_rs qidors_xml(
                "/studies", "StudyInstanceUID=no_data",
                dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

    BOOST_CHECK_EQUAL(qidors_xml.get_contenttype(),
                      dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

    dopamine::services::Qido_rs qidors_json(
                "/studies", "StudyInstanceUID=no_data",
                dopamine::services::MIME_TYPE_APPLICATION_JSON);

    BOOST_CHECK_EQUAL(qidors_json.get_contenttype(),
                      dopamine::services::MIME_TYPE_APPLICATION_JSON);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Study level)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudy_XML, TestDataRequest)
{
    std::string const pathinfo = "/studies";

    // PS3.18: 6.7.1.2.1.1 Study Matching -
    // Table 6.7.1-1. QIDO-RS STUDY Search Query Keys
    std::vector<std::string> queries =
    {
        "StudyDate=20130328",
        "StudyTime=101009",
//        "AccessionNumber=*",              // Cannot be test => Null value
//        "ModalitiesInStudy=*",            // Cannot be test => Not present
        "ReferringPhysicianName=Gregory^House",
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

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_DICOMXML,
                                               "");
        std::string const response_xml = qidors_xml.get_response();

        // Check response
        mimetic::MimeEntity entity = to_MIME_message(response_xml,
                                                     qidors_xml.get_boundary());

        // Check Header
        mimetic::Header& h = entity.header();
        BOOST_CHECK(h.contentType().isMultipart());
        std::string content_type = h.contentType().str();
        BOOST_CHECK(content_type.find(
                        dopamine::services::MIME_TYPE_MULTIPART_RELATED) !=
                    std::string::npos);

        // Check each parts
        mimetic::MimeEntityList& parts = entity.body().parts();
        BOOST_CHECK_EQUAL(parts.size(), 1);
        for(mimetic::MimeEntityList::iterator mbit = parts.begin();
            mbit != parts.end(); ++mbit)
        {
            // check Header
            mimetic::Header& header = (*mbit)->header();
            std::stringstream contenttypestream;
            contenttypestream << header.contentType();

            BOOST_REQUIRE_EQUAL(
                        contenttypestream.str(),
                        dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

            // check Body
            mimetic::Body& body = (*mbit)->body();

            // remove the ended boundary
            std::string temp(body.c_str(), body.size());
            temp = temp.substr(0, temp.rfind("\n\n--"));

            // remove ended '\n'
            while (temp[temp.size()-1] == '\n')
            {
                temp = temp.substr(0, temp.rfind("\n"));
            }

            BOOST_CHECK(temp.find(STUDY_INSTANCE_UID_01_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find("STUDY") != std::string::npos);

            // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
            for (std::string attribute : mandatory_study_attributes)
            {
                if (temp.find(attribute) == std::string::npos)
                {
                    std::stringstream streamerror;
                    streamerror << "Missing mandatory attribute: "
                                << attribute;
                    BOOST_THROW_EXCEPTION(
                        dopamine::ExceptionPACS(streamerror.str()));
                }
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
BOOST_FIXTURE_TEST_CASE(RequestStudy_JSON, TestDataRequest)
{
    std::string const pathinfo = "/studies";

    // PS3.18: 6.7.1.2.1.1 Study Matching -
    // Table 6.7.1-1. QIDO-RS STUDY Search Query Keys
    std::vector<std::string> queries =
    {
        "StudyDate=20130328",
        "StudyTime=101009",
//        "AccessionNumber=*",              // Cannot be test => Null value
//        "ModalitiesInStudy=*",            // Cannot be test => Not present
        "ReferringPhysicianName=Gregory^House",
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

    for (std::string const query : queries)
    {
        // Perform query and get JSON response
        dopamine::services::Qido_rs qidors_json(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_JSON, "");
        std::string const response_json = qidors_json.get_response();

        // Check response
        BOOST_REQUIRE(response_json != "");
        BOOST_CHECK(response_json.find(STUDY_INSTANCE_UID_01_01) !=
                    std::string::npos);

        // Convert response to BSON Object
        std::stringstream response;
        response << "{ arrayjson : " << response_json << " }";
        mongo::BSONObj objectjson = mongo::fromjson(response.str());
        BOOST_REQUIRE_EQUAL(objectjson["arrayjson"].Array().size(), 1);

        mongo::BSONObj const object =
                objectjson["arrayjson"].Array()[0].Obj();

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : mandatory_study_attributes)
        {
            if (object.hasField(attribute) == false)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: "
                            << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Series level)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudySeries_XML, TestDataRequest)
{
    std::stringstream streampathinfo;
    streampathinfo << "/studies/" << STUDY_INSTANCE_UID_01_01 << "/series";
    std::string const pathinfo = streampathinfo.str();

    // PS3.18: 6.7.1.2.1.2 Series Matching -
    // Table 6.7.1-1a. QIDO-RS SERIES Search Query Keys
    std::vector<std::string> queries =
    {
        "Modality=MR",
        "SeriesInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1",
        "SeriesNumber=196609",
//        "PerformedProcedureStepStartDate=*",                  // Not present
//        "PerformedProcedureStepStartTime=*",                  // Not present
//        "RequestAttributeSequence.ScheduledProcedureStepID=*",// Not present
//        "RequestAttributeSequence.RequestedProcedureID=*",    // Not present

        // PS3.18: 6.7.1.1.1 {attributeID} encoding rules
        "00080060=M?",
        "0020000e=2.16.756.5.5.100.3611280983.20092.1364462458.1",
        "0020000E=2.16.756.5.5.100.3611280983.20092.1364462458.1",

        // Multi parameters
        "0020000e=2.16.756.5.5.100.3611280983.20092.1364462458.1&Modality=MR&SeriesNumber=196609"
    };

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_DICOMXML, "");
        std::string const response_xml = qidors_xml.get_response();

        // Check response
        mimetic::MimeEntity entity =
                to_MIME_message(response_xml, qidors_xml.get_boundary());

        // Check Header
        mimetic::Header& h = entity.header();
        BOOST_CHECK(h.contentType().isMultipart());
        std::string content_type = h.contentType().str();
        BOOST_CHECK(content_type.find(
                        dopamine::services::MIME_TYPE_MULTIPART_RELATED) !=
                    std::string::npos);

        // Check each parts
        mimetic::MimeEntityList& parts = entity.body().parts();
        BOOST_CHECK_EQUAL(parts.size(), 1);
        for(mimetic::MimeEntityList::iterator mbit = parts.begin();
            mbit != parts.end(); ++mbit)
        {
            // check Header
            mimetic::Header& header = (*mbit)->header();
            std::stringstream contenttypestream;
            contenttypestream << header.contentType();

            BOOST_REQUIRE_EQUAL(
                        contenttypestream.str(),
                        dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

            // check Body
            mimetic::Body& body = (*mbit)->body();

            // remove the ended boundary
            std::string temp(body.c_str(), body.size());
            temp = temp.substr(0, temp.rfind("\n\n--"));

            // remove ended '\n'
            while (temp[temp.size()-1] == '\n')
            {
                temp = temp.substr(0, temp.rfind("\n"));
            }

            BOOST_CHECK(temp.find(STUDY_INSTANCE_UID_01_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find(SERIES_INSTANCE_UID_01_01_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find("SERIES") != std::string::npos);

            // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
            for (std::string attribute : mandatory_series_attributes)
            {
                if (temp.find(attribute) == std::string::npos)
                {
                    std::stringstream streamerror;
                    streamerror << "Missing mandatory attribute: "
                                << attribute;
                    BOOST_THROW_EXCEPTION(
                        dopamine::ExceptionPACS(streamerror.str()));
                }
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Series level)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudySeries_JSON, TestDataRequest)
{
    std::stringstream streampathinfo;
    streampathinfo << "/studies/" << STUDY_INSTANCE_UID_01_01 << "/series";
    std::string const pathinfo = streampathinfo.str();

    // PS3.18: 6.7.1.2.1.2 Series Matching -
    // Table 6.7.1-1a. QIDO-RS SERIES Search Query Keys
    std::vector<std::string> queries =
    {
        "Modality=MR",
        "SeriesInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1",
        "SeriesNumber=196609",
//        "PerformedProcedureStepStartDate=*",                  // Not present
//        "PerformedProcedureStepStartTime=*",                  // Not present
//        "RequestAttributeSequence.ScheduledProcedureStepID=*",// Not present
//        "RequestAttributeSequence.RequestedProcedureID=*",    // Not present

        // PS3.18: 6.7.1.1.1 {attributeID} encoding rules
        "00080060=M?",
        "0020000e=2.16.756.5.5.100.3611280983.20092.1364462458.1",
        "0020000E=2.16.756.5.5.100.3611280983.20092.1364462458.1",

        // Multi parameters
        "0020000e=2.16.756.5.5.100.3611280983.20092.1364462458.1&Modality=MR&SeriesNumber=196609"
    };

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_json(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_JSON, "");
        std::string const response_json = qidors_json.get_response();

        // Convert response to BSON Object
        std::stringstream response;
        response << "{ arrayjson : " << response_json << " }";
        mongo::BSONObj objectjson = mongo::fromjson(response.str());

        // Check response
        BOOST_REQUIRE(response_json != "");
        BOOST_CHECK(response_json.find(STUDY_INSTANCE_UID_01_01) !=
                    std::string::npos);
        BOOST_CHECK(response_json.find(SERIES_INSTANCE_UID_01_01_01) !=
                    std::string::npos);
        BOOST_REQUIRE_EQUAL(objectjson["arrayjson"].Array().size(), 1);

        mongo::BSONObj const object =
                objectjson["arrayjson"].Array()[0].Obj();

        // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
        for (std::string attribute : mandatory_series_attributes)
        {
            if (object.hasField(attribute) == false)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: "
                            << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
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
    std::string const pathinfo = "/series";

    // PS3.18: 6.7.1.2.1.2 Series Matching -
    // Table 6.7.1-1a. QIDO-RS SERIES Search Query Keys
    std::vector<std::string> queries =
    {
        "Modality=MR",
        "SeriesInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1",
        "SeriesNumber=196609",
//        "PerformedProcedureStepStartDate=*",                  // Not present
//        "PerformedProcedureStepStartTime=*",                  // Not present
//        "RequestAttributeSequence.ScheduledProcedureStepID=*",// Not present
//        "RequestAttributeSequence.RequestedProcedureID=*",    // Not present

        // PS3.18: 6.7.1.1.1 {attributeID} encoding rules
        "00080060=M?",
        "0020000e=2.16.756.5.5.100.3611280983.20092.1364462458.1",
        "0020000E=2.16.756.5.5.100.3611280983.20092.1364462458.1",

        // Multi parameters
        "0020000e=2.16.756.5.5.100.3611280983.20092.1364462458.1&Modality=MR&SeriesNumber=196609"
    };

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_DICOMXML, "");
        std::string const response_xml = qidors_xml.get_response();

        // Check response
        mimetic::MimeEntity entity =
                to_MIME_message(response_xml, qidors_xml.get_boundary());

        // Check Header
        mimetic::Header& h = entity.header();
        BOOST_CHECK(h.contentType().isMultipart());
        std::string content_type = h.contentType().str();
        BOOST_CHECK(content_type.find(
                        dopamine::services::MIME_TYPE_MULTIPART_RELATED) !=
                    std::string::npos);

        // Check each parts
        mimetic::MimeEntityList& parts = entity.body().parts();
        BOOST_CHECK_GE(parts.size(), 1);
        for(mimetic::MimeEntityList::iterator mbit = parts.begin();
            mbit != parts.end(); ++mbit)
        {
            // check Header
            mimetic::Header& header = (*mbit)->header();
            std::stringstream contenttypestream;
            contenttypestream << header.contentType();

            BOOST_REQUIRE_EQUAL(
                        contenttypestream.str(),
                        dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

            // check Body
            mimetic::Body& body = (*mbit)->body();

            // remove the ended boundary
            std::string temp(body.c_str(), body.size());
            temp = temp.substr(0, temp.rfind("\n\n--"));

            // remove ended '\n'
            while (temp[temp.size()-1] == '\n')
            {
                temp = temp.substr(0, temp.rfind("\n"));
            }

            if (query != "Modality=MR" && query != "00080060=M?")
            {
                BOOST_CHECK(temp.find(STUDY_INSTANCE_UID_01_01) !=
                            std::string::npos);
                BOOST_CHECK(temp.find(SERIES_INSTANCE_UID_01_01_01) !=
                            std::string::npos);
            }
            BOOST_CHECK(temp.find("SERIES") != std::string::npos);

            // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
            for (std::string attribute : mandatory_study_attributes)
            {
                if (temp.find(attribute) == std::string::npos)
                {
                    std::stringstream streamerror;
                    streamerror << "Missing mandatory attribute: "
                                << attribute;
                    BOOST_THROW_EXCEPTION(
                        dopamine::ExceptionPACS(streamerror.str()));
                }
            }

            // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
            for (std::string attribute : mandatory_series_attributes)
            {
                if (temp.find(attribute) == std::string::npos)
                {
                    std::stringstream streamerror;
                    streamerror << "Missing mandatory attribute: "
                                << attribute;
                    BOOST_THROW_EXCEPTION(
                        dopamine::ExceptionPACS(streamerror.str()));
                }
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Series level)
 */
BOOST_FIXTURE_TEST_CASE(RequestSeries_JSON, TestDataRequest)
{
    std::string const pathinfo = "/series";

    // PS3.18: 6.7.1.2.1.2 Series Matching -
    // Table 6.7.1-1a. QIDO-RS SERIES Search Query Keys
    std::vector<std::string> queries =
    {
        "Modality=MR",
        "SeriesInstanceUID=2.16.756.5.5.100.3611280983.20092.1364462458.1",
        "SeriesNumber=196609",
//        "PerformedProcedureStepStartDate=*",                  // Not present
//        "PerformedProcedureStepStartTime=*",                  // Not present
//        "RequestAttributeSequence.ScheduledProcedureStepID=*",// Not present
//        "RequestAttributeSequence.RequestedProcedureID=*",    // Not present

        // PS3.18: 6.7.1.1.1 {attributeID} encoding rules
        "00080060=M?",
        "0020000e=2.16.756.5.5.100.3611280983.20092.1364462458.1",
        "0020000E=2.16.756.5.5.100.3611280983.20092.1364462458.1",

        // Multi parameters
        "0020000e=2.16.756.5.5.100.3611280983.20092.1364462458.1&Modality=MR&SeriesNumber=196609"
    };

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_json(
                    pathinfo, query,
                     dopamine::services::MIME_TYPE_APPLICATION_JSON, "");
        std::string const response_json = qidors_json.get_response();

        // Convert response to BSON Object
        std::stringstream response;
        response << "{ arrayjson : " << response_json << " }";
        mongo::BSONObj objectjson = mongo::fromjson(response.str());

        // Check response
        BOOST_REQUIRE(response_json != "");
        BOOST_CHECK(response_json.find(STUDY_INSTANCE_UID_01_01) !=
                    std::string::npos);
        BOOST_CHECK(response_json.find(SERIES_INSTANCE_UID_01_01_01) !=
                    std::string::npos);
        BOOST_REQUIRE_GE(objectjson["arrayjson"].Array().size(), 1);

        mongo::BSONObj const object =
                objectjson["arrayjson"].Array()[0].Obj();

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : mandatory_study_attributes)
        {
            if (object.hasField(attribute) == false)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: "
                            << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
            }
        }

        // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
        for (std::string attribute : mandatory_series_attributes)
        {
            if (object.hasField(attribute) == false)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: "
                            << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
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
    std::stringstream streampathinfo;
    streampathinfo << "/studies/" << STUDY_INSTANCE_UID_01_01
                   << "/series/" << SERIES_INSTANCE_UID_01_01_01 << "/instances";
    std::string const pathinfo = streampathinfo.str();

    // PS3.18: 6.7.1.2.1.3 Instance Matching -
    // Table 6.7.1-1b. QIDO-RS INSTANCE Search Query Keys
    std::vector<std::string> queries =
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

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_DICOMXML, "");
        std::string const response_xml = qidors_xml.get_response();

        // Check response
        mimetic::MimeEntity entity =
                to_MIME_message(response_xml, qidors_xml.get_boundary());

        // Check Header
        mimetic::Header& h = entity.header();
        BOOST_CHECK(h.contentType().isMultipart());
        std::string content_type = h.contentType().str();
        BOOST_CHECK(content_type.find(
                        dopamine::services::MIME_TYPE_MULTIPART_RELATED) !=
                    std::string::npos);

        // Check each parts
        mimetic::MimeEntityList& parts = entity.body().parts();
        BOOST_CHECK_EQUAL(parts.size(), 1);
        for(mimetic::MimeEntityList::iterator mbit = parts.begin();
            mbit != parts.end(); ++mbit)
        {
            // check Header
            mimetic::Header& header = (*mbit)->header();
            std::stringstream contenttypestream;
            contenttypestream << header.contentType();

            BOOST_REQUIRE_EQUAL(
                        contenttypestream.str(),
                        dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

            // check Body
            mimetic::Body& body = (*mbit)->body();

            // remove the ended boundary
            std::string temp(body.c_str(), body.size());
            temp = temp.substr(0, temp.rfind("\n\n--"));

            // remove ended '\n'
            while (temp[temp.size()-1] == '\n')
            {
                temp = temp.substr(0, temp.rfind("\n"));
            }

            BOOST_CHECK(temp.find(STUDY_INSTANCE_UID_01_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find(SERIES_INSTANCE_UID_01_01_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find(SOP_INSTANCE_UID_01_01_01_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find("IMAGE") != std::string::npos);

            // See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
            for (std::string attribute : mandatory_instance_attributes)
            {
                if (temp.find(attribute) == std::string::npos)
                {
                    std::stringstream streamerror;
                    streamerror << "Missing mandatory attribute: " << attribute;
                    BOOST_THROW_EXCEPTION(
                        dopamine::ExceptionPACS(streamerror.str()));
                }
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Instance level)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudySeriesInstance_JSON, TestDataRequest)
{
    std::stringstream streampathinfo;
    streampathinfo << "/studies/" << STUDY_INSTANCE_UID_01_01
                   << "/series/" << SERIES_INSTANCE_UID_01_01_01 << "/instances";
    std::string const pathinfo = streampathinfo.str();

    // PS3.18: 6.7.1.2.1.3 Instance Matching -
    // Table 6.7.1-1b. QIDO-RS INSTANCE Search Query Keys
    std::vector<std::string> queries =
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

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_json(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_JSON, "");
        std::string const response_json = qidors_json.get_response();

        // Convert response to BSON Object
        std::stringstream response;
        response << "{ arrayjson : " << response_json << " }";
        mongo::BSONObj objectjson = mongo::fromjson(response.str());

        // Check response
        BOOST_REQUIRE(response_json != "");
        BOOST_CHECK(response_json.find(STUDY_INSTANCE_UID_01_01) !=
                    std::string::npos);
        BOOST_CHECK(response_json.find(SERIES_INSTANCE_UID_01_01_01) !=
                    std::string::npos);
        BOOST_CHECK(response_json.find(SOP_INSTANCE_UID_01_01_01_01) !=
                    std::string::npos);
        BOOST_REQUIRE_EQUAL(objectjson["arrayjson"].Array().size(), 1);

        mongo::BSONObj const object = objectjson["arrayjson"].Array()[0].Obj();

        // See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
        for (std::string attribute : mandatory_instance_attributes)
        {
            if (object.hasField(attribute) == false)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
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
    std::stringstream streampathinfo;
    streampathinfo << "/studies/" << STUDY_INSTANCE_UID_01_01 << "/instances";
    std::string const pathinfo = streampathinfo.str();

    // PS3.18: 6.7.1.2.1.3 Instance Matching -
    // Table 6.7.1-1b. QIDO-RS INSTANCE Search Query Keys
    std::vector<std::string> queries =
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

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_DICOMXML, "");
        std::string const response_xml = qidors_xml.get_response();

        // Check response
        mimetic::MimeEntity entity =
                to_MIME_message(response_xml, qidors_xml.get_boundary());

        // Check Header
        mimetic::Header& h = entity.header();
        BOOST_CHECK(h.contentType().isMultipart());
        std::string content_type = h.contentType().str();
        BOOST_CHECK(content_type.find(
                        dopamine::services::MIME_TYPE_MULTIPART_RELATED) !=
                    std::string::npos);

        // Check each parts
        mimetic::MimeEntityList& parts = entity.body().parts();
        BOOST_CHECK_EQUAL(parts.size(), 1);
        for(mimetic::MimeEntityList::iterator mbit = parts.begin();
            mbit != parts.end(); ++mbit)
        {
            // check Header
            mimetic::Header& header = (*mbit)->header();
            std::stringstream contenttypestream;
            contenttypestream << header.contentType();

            BOOST_REQUIRE_EQUAL(
                        contenttypestream.str(),
                        dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

            // check Body
            mimetic::Body& body = (*mbit)->body();

            // remove the ended boundary
            std::string temp(body.c_str(), body.size());
            temp = temp.substr(0, temp.rfind("\n\n--"));

            // remove ended '\n'
            while (temp[temp.size()-1] == '\n')
            {
                temp = temp.substr(0, temp.rfind("\n"));
            }

            BOOST_CHECK(temp.find(STUDY_INSTANCE_UID_01_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find(SERIES_INSTANCE_UID_01_01_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find(SOP_INSTANCE_UID_01_01_01_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find("IMAGE") != std::string::npos);

            // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
            for (std::string attribute : mandatory_series_attributes)
            {
                if (temp.find(attribute) == std::string::npos)
                {
                    std::stringstream streamerror;
                    streamerror << "Missing mandatory attribute: " << attribute;
                    BOOST_THROW_EXCEPTION(
                        dopamine::ExceptionPACS(streamerror.str()));
                }
            }

            // See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
            for (std::string attribute : mandatory_instance_attributes)
            {
                if (temp.find(attribute) == std::string::npos)
                {
                    std::stringstream streamerror;
                    streamerror << "Missing mandatory attribute: " << attribute;
                    BOOST_THROW_EXCEPTION(
                        dopamine::ExceptionPACS(streamerror.str()));
                }
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Instance level)
 */
BOOST_FIXTURE_TEST_CASE(RequestStudyInstance_JSON, TestDataRequest)
{
    std::stringstream streampathinfo;
    streampathinfo << "/studies/" << STUDY_INSTANCE_UID_01_01 << "/instances";
    std::string const pathinfo = streampathinfo.str();

    // PS3.18: 6.7.1.2.1.3 Instance Matching -
    // Table 6.7.1-1b. QIDO-RS INSTANCE Search Query Keys
    std::vector<std::string> queries =
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

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_json(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_JSON, "");
        std::string const response_json = qidors_json.get_response();

        // Convert response to BSON Object
        std::stringstream response;
        response << "{ arrayjson : " << response_json << " }";
        mongo::BSONObj objectjson = mongo::fromjson(response.str());

        // Check response
        BOOST_REQUIRE(response_json != "");
        BOOST_CHECK(response_json.find(STUDY_INSTANCE_UID_01_01) !=
                    std::string::npos);
        BOOST_CHECK(response_json.find(SERIES_INSTANCE_UID_01_01_01) !=
                    std::string::npos);
        BOOST_CHECK(response_json.find(SOP_INSTANCE_UID_01_01_01_01) !=
                    std::string::npos);
        BOOST_REQUIRE_EQUAL(objectjson["arrayjson"].Array().size(), 1);

        mongo::BSONObj const object = objectjson["arrayjson"].Array()[0].Obj();

        // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
        for (std::string attribute : mandatory_series_attributes)
        {
            if (object.hasField(attribute) == false)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
            }
        }

        // See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
        for (std::string attribute : mandatory_instance_attributes)
        {
            if (object.hasField(attribute) == false)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
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
    std::string const pathinfo = "/instances";

    // PS3.18: 6.7.1.2.1.3 Instance Matching -
    // Table 6.7.1-1b. QIDO-RS INSTANCE Search Query Keys
    std::vector<std::string> queries =
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

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_DICOMXML, "");
        std::string const response_xml = qidors_xml.get_response();

        // Check response
        mimetic::MimeEntity entity =
                to_MIME_message(response_xml, qidors_xml.get_boundary());

        // Check Header
        mimetic::Header& h = entity.header();
        BOOST_CHECK(h.contentType().isMultipart());
        std::string content_type = h.contentType().str();
        BOOST_CHECK(content_type.find(
                        dopamine::services::MIME_TYPE_MULTIPART_RELATED) !=
                    std::string::npos);

        // Check each parts
        mimetic::MimeEntityList& parts = entity.body().parts();
        BOOST_CHECK_GE(parts.size(), 1);
        for(mimetic::MimeEntityList::iterator mbit = parts.begin();
            mbit != parts.end(); ++mbit)
        {
            // check Header
            mimetic::Header& header = (*mbit)->header();
            std::stringstream contenttypestream;
            contenttypestream << header.contentType();

            BOOST_REQUIRE_EQUAL(
                        contenttypestream.str(),
                        dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

            // check Body
            mimetic::Body& body = (*mbit)->body();

            // remove the ended boundary
            std::string temp(body.c_str(), body.size());
            temp = temp.substr(0, temp.rfind("\n\n--"));

            // remove ended '\n'
            while (temp[temp.size()-1] == '\n')
            {
                temp = temp.substr(0, temp.rfind("\n"));
            }

            if (query != "SOPClassUID=1.2.840.10008.5.1.4.1.1.4" &&
                query != "00080016=1.2.840.10008.5.1.4.1.1.4" &&
                query != "InstanceNumber=1" &&
                query != "00200013=1")
            {
                BOOST_CHECK(temp.find(STUDY_INSTANCE_UID_01_01) !=
                            std::string::npos);
                BOOST_CHECK(temp.find(SERIES_INSTANCE_UID_01_01_01) !=
                            std::string::npos);
                BOOST_CHECK(temp.find(SOP_INSTANCE_UID_01_01_01_01) !=
                            std::string::npos);
            }
            BOOST_CHECK_EQUAL(temp.find("IMAGE") != std::string::npos, true);

            // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
            for (std::string attribute : mandatory_study_attributes)
            {
                if (temp.find(attribute) == std::string::npos)
                {
                    std::stringstream streamerror;
                    streamerror << "Missing mandatory attribute: "
                                << attribute;
                    BOOST_THROW_EXCEPTION(
                        dopamine::ExceptionPACS(streamerror.str()));
                }
            }

            // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
            for (std::string attribute : mandatory_series_attributes)
            {
                if (temp.find(attribute) == std::string::npos)
                {
                    std::stringstream streamerror;
                    streamerror << "Missing mandatory attribute: "
                                << attribute;
                    BOOST_THROW_EXCEPTION(
                        dopamine::ExceptionPACS(streamerror.str()));
                }
            }

            // See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
            for (std::string attribute : mandatory_instance_attributes)
            {
                if (temp.find(attribute) == std::string::npos)
                {
                    std::stringstream streamerror;
                    streamerror << "Missing mandatory attribute: "
                                << attribute;
                    BOOST_THROW_EXCEPTION(
                        dopamine::ExceptionPACS(streamerror.str()));
                }
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Instance level)
 */
BOOST_FIXTURE_TEST_CASE(RequestInstance_JSON, TestDataRequest)
{
    std::string const pathinfo = "/instances";

    // PS3.18: 6.7.1.2.1.3 Instance Matching -
    // Table 6.7.1-1b. QIDO-RS INSTANCE Search Query Keys
    std::vector<std::string> queries =
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

    for (std::string const query : queries)
    {
        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_json(
                    pathinfo, query,
                    dopamine::services::MIME_TYPE_APPLICATION_JSON, "");
        std::string const response_json = qidors_json.get_response();

        // Convert response to BSON Object
        std::stringstream response;
        response << "{ arrayjson : " << response_json << " }";
        mongo::BSONObj objectjson = mongo::fromjson(response.str());

        // Check response
        BOOST_REQUIRE(response_json != "");
        BOOST_CHECK(response_json.find(STUDY_INSTANCE_UID_01_01) !=
                    std::string::npos);
        BOOST_CHECK(response_json.find(SERIES_INSTANCE_UID_01_01_01) !=
                    std::string::npos);
        BOOST_CHECK(response_json.find(SOP_INSTANCE_UID_01_01_01_01) !=
                    std::string::npos);
        BOOST_REQUIRE_GE(objectjson["arrayjson"].Array().size(), 1);

        mongo::BSONObj const object = objectjson["arrayjson"].Array()[0].Obj();

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : mandatory_study_attributes)
        {
            if (object.hasField(attribute) == false)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
            }
        }

        // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
        for (std::string attribute : mandatory_series_attributes)
        {
            if (object.hasField(attribute) == false)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
            }
        }

        // See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
        for (std::string attribute : mandatory_instance_attributes)
        {
            if (object.hasField(attribute) == false)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
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
    std::string const pathinfo = "/studies";

    // PS3.18: 6.7.1.2.1.1 Study Matching -
    // Table 6.7.1-1. QIDO-RS STUDY Search Query Keys
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

    std::stringstream query;
    query << "StudyInstanceUID=" << STUDY_INSTANCE_UID_01_01
          << "&PatientName=Doe^Jane";

    for (std::string const field : field_to_test)
    {
        query << "&includefield=" << field;
    }

    // Perform query and get XML response
    dopamine::services::Qido_rs qidors_xml(
                pathinfo, query.str(),
                dopamine::services::MIME_TYPE_APPLICATION_DICOMXML, "");
    std::string const response_xml = qidors_xml.get_response();

    // Check response
    mimetic::MimeEntity entity =
            to_MIME_message(response_xml, qidors_xml.get_boundary());

    // Check Header
    mimetic::Header& h = entity.header();
    BOOST_CHECK(h.contentType().isMultipart());
    std::string content_type = h.contentType().str();
    BOOST_CHECK(content_type.find(
                    dopamine::services::MIME_TYPE_MULTIPART_RELATED) !=
                std::string::npos);

    // Check each parts
    mimetic::MimeEntityList& parts = entity.body().parts();
    BOOST_CHECK_EQUAL(parts.size(), 1);
    for(mimetic::MimeEntityList::iterator mbit = parts.begin();
        mbit != parts.end(); ++mbit)
    {
        // check Header
        mimetic::Header& header = (*mbit)->header();
        std::stringstream contenttypestream;
        contenttypestream << header.contentType();

        BOOST_REQUIRE_EQUAL(
                    contenttypestream.str(),
                    dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

        // check Body
        mimetic::Body& body = (*mbit)->body();

        // remove the ended boundary
        std::string temp(body.c_str(), body.size());
        temp = temp.substr(0, temp.rfind("\n\n--"));

        // remove ended '\n'
        while (temp[temp.size()-1] == '\n')
        {
            temp = temp.substr(0, temp.rfind("\n"));
        }

        BOOST_CHECK(temp.find(STUDY_INSTANCE_UID_01_01) !=
                    std::string::npos);
        BOOST_CHECK(temp.find("STUDY") != std::string::npos);

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : mandatory_study_attributes)
        {
            if (temp.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
            }
        }

        for (std::string const field : field_to_test)
        {
            if (temp.find(field) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing include field: " << field;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
            }
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request Limit
 */
BOOST_FIXTURE_TEST_CASE(RequestLimit_XML, TestDataRequest)
{
    std::string const pathinfo = "/studies";

    std::vector<unsigned int> limits = { 3, 2, 1, 4 };

    for (unsigned int limit : limits)
    {
        std::stringstream query;
        query << "StudyInstanceUID=" << STUDY_INSTANCE_UID_02_01;
        query << "&limit=" << limit;

        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(
                    pathinfo, query.str(),
                    dopamine::services::MIME_TYPE_APPLICATION_DICOMXML, "");
        std::string const response_xml = qidors_xml.get_response();

        // Check response
        mimetic::MimeEntity entity =
                to_MIME_message(response_xml, qidors_xml.get_boundary());

        // Check Header
        mimetic::Header& h = entity.header();
        BOOST_CHECK(h.contentType().isMultipart());
        std::string content_type = h.contentType().str();
        BOOST_CHECK(content_type.find(
                        dopamine::services::MIME_TYPE_MULTIPART_RELATED) !=
                    std::string::npos);

        // Check each parts
        mimetic::MimeEntityList& parts = entity.body().parts();
        if (limit > 3)
        {
            BOOST_CHECK_EQUAL(parts.size(), 3);
        }
        else
        {
            BOOST_CHECK_EQUAL(parts.size(), limit);
        }
        for(mimetic::MimeEntityList::iterator mbit = parts.begin();
            mbit != parts.end(); ++mbit)
        {
            // check Header
            mimetic::Header& header = (*mbit)->header();
            std::stringstream contenttypestream;
            contenttypestream << header.contentType();

            BOOST_REQUIRE_EQUAL(
                        contenttypestream.str(),
                        dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

            // check Body
            mimetic::Body& body = (*mbit)->body();

            // remove the ended boundary
            std::string temp(body.c_str(), body.size());
            temp = temp.substr(0, temp.rfind("\n\n--"));

            // remove ended '\n'
            while (temp[temp.size()-1] == '\n')
            {
                temp = temp.substr(0, temp.rfind("\n"));
            }

            BOOST_CHECK(temp.find(STUDY_INSTANCE_UID_02_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find("STUDY") != std::string::npos);
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request Offset
 */
BOOST_FIXTURE_TEST_CASE(RequestOffset_XML, TestDataRequest)
{
    std::string const pathinfo = "/studies";

    std::vector<unsigned int> offsets = { 0, 1, 2, 3 };

    for (unsigned int offset : offsets)
    {
        std::stringstream query;
        query << "StudyInstanceUID=" << STUDY_INSTANCE_UID_02_01;
        query << "&offset=" << offset;

        // Perform query and get XML response
        dopamine::services::Qido_rs qidors_xml(
                    pathinfo, query.str(),
                    dopamine::services::MIME_TYPE_APPLICATION_DICOMXML, "");
        std::string const response_xml = qidors_xml.get_response();

        // Check response
        mimetic::MimeEntity entity =
                to_MIME_message(response_xml, qidors_xml.get_boundary());

        // Check Header
        mimetic::Header& h = entity.header();
        BOOST_CHECK(h.contentType().isMultipart());
        std::string content_type = h.contentType().str();
        BOOST_CHECK(content_type.find(
                        dopamine::services::MIME_TYPE_MULTIPART_RELATED) !=
                    std::string::npos);

        // Check each parts
        mimetic::MimeEntityList& parts = entity.body().parts();
        BOOST_CHECK_EQUAL(parts.size(), (3 - offset));
        for(mimetic::MimeEntityList::iterator mbit = parts.begin();
            mbit != parts.end(); ++mbit)
        {
            // check Header
            mimetic::Header& header = (*mbit)->header();
            std::stringstream contenttypestream;
            contenttypestream << header.contentType();

            BOOST_REQUIRE_EQUAL(
                        contenttypestream.str(),
                        dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

            // check Body
            mimetic::Body& body = (*mbit)->body();

            // remove the ended boundary
            std::string temp(body.c_str(), body.size());
            temp = temp.substr(0, temp.rfind("\n\n--"));

            // remove ended '\n'
            while (temp[temp.size()-1] == '\n')
            {
                temp = temp.substr(0, temp.rfind("\n"));
            }

            BOOST_CHECK(temp.find(STUDY_INSTANCE_UID_02_01) !=
                        std::string::npos);
            BOOST_CHECK(temp.find("STUDY") != std::string::npos);
        }
    }
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs request (Study level)
 */
BOOST_FIXTURE_TEST_CASE(Request_Range, TestDataRequest)
{
    std::string const pathinfo = "/studies";

    // PS3.18: 6.7.1.2.1.1 Study Matching -
    // Table 6.7.1-1. QIDO-RS STUDY Search Query Keys
    std::string query = "StudyDate=20130327-20130329";

    // Perform query and get XML response
    dopamine::services::Qido_rs qidors_xml(
                pathinfo, query,
                dopamine::services::MIME_TYPE_APPLICATION_DICOMXML, "");
    std::string const response_xml = qidors_xml.get_response();

    // Check response
    mimetic::MimeEntity entity =
            to_MIME_message(response_xml, qidors_xml.get_boundary());

    // Check Header
    mimetic::Header& h = entity.header();
    BOOST_CHECK(h.contentType().isMultipart());
    std::string content_type = h.contentType().str();
    BOOST_CHECK(content_type.find(
                    dopamine::services::MIME_TYPE_MULTIPART_RELATED) !=
                std::string::npos);

    // Check each parts
    mimetic::MimeEntityList& parts = entity.body().parts();
    BOOST_CHECK_EQUAL(parts.size(), 1);
    for(mimetic::MimeEntityList::iterator mbit = parts.begin();
        mbit != parts.end(); ++mbit)
    {
        // check Header
        mimetic::Header& header = (*mbit)->header();
        std::stringstream contenttypestream;
        contenttypestream << header.contentType();

        BOOST_REQUIRE_EQUAL(
                    contenttypestream.str(),
                    dopamine::services::MIME_TYPE_APPLICATION_DICOMXML);

        // check Body
        mimetic::Body& body = (*mbit)->body();

        // remove the ended boundary
        std::string temp(body.c_str(), body.size());
        temp = temp.substr(0, temp.rfind("\n\n--"));

        // remove ended '\n'
        while (temp[temp.size()-1] == '\n')
        {
            temp = temp.substr(0, temp.rfind("\n"));
        }

        BOOST_CHECK(temp.find(STUDY_INSTANCE_UID_01_01) !=
                    std::string::npos);
        BOOST_CHECK(temp.find("STUDY") != std::string::npos);

        // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        for (std::string attribute : mandatory_study_attributes)
        {
            if (temp.find(attribute) == std::string::npos)
            {
                std::stringstream streamerror;
                streamerror << "Missing mandatory attribute: " << attribute;
                BOOST_THROW_EXCEPTION(
                    dopamine::ExceptionPACS(streamerror.str()));
            }
        }
    }
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Bad query
 */
BOOST_FIXTURE_TEST_CASE(RequestBadQuery, TestDataRequest)
{
    std::vector<std::string> pathinfo_to_test =
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

    for (auto pathinfo : pathinfo_to_test)
    {
        BOOST_CHECK_EXCEPTION(
                    dopamine::services::Qido_rs(pathinfo, ""),
                    dopamine::services::WebServiceException,
                    [] (dopamine::services::WebServiceException const exc)
                        { return (exc.status() == 400 &&
                                  exc.statusmessage() == "Bad Request"); });
    }
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Bad tag
 */
BOOST_FIXTURE_TEST_CASE(RequestBadTag, TestDataRequest)
{
    std::string const pathinfo = "/instances";

    std::stringstream query;
    query << "StudyInstanceUID=" << STUDY_INSTANCE_UID_02_01;
    query << "&NotADICOMTag=badvalue";

    BOOST_CHECK_EXCEPTION(
                dopamine::services::Qido_rs(pathinfo, query.str()),
                dopamine::services::WebServiceException,
                [] (dopamine::services::WebServiceException const exc)
                    { return (exc.status() == 400 &&
                              exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Fuzzy Matching (not implemented)
 */
BOOST_FIXTURE_TEST_CASE(RequestFuzzyMatching, TestDataRequest)
{
    std::string const pathinfo = "/studies";

    std::stringstream query;
    query << "StudyInstanceUID=" << STUDY_INSTANCE_UID_02_01;
    query << "&fuzzymatching=true";

    BOOST_CHECK_EXCEPTION(
                dopamine::services::Qido_rs(pathinfo, query.str()),
                dopamine::services::WebServiceException,
                [] (dopamine::services::WebServiceException const exc)
                    { return (exc.status() == 299 &&
                              exc.statusmessage() == "Warning"); });
}
