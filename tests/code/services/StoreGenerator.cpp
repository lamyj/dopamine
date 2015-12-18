/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleStoreGenerator
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/message/CStoreRequest.h>
#include <dcmtkpp/message/CStoreResponse.h>

#include "ServicesTestClass.h"
#include "services/StoreGenerator.h"

class TestDataGenerator_constraint : public ServicesTestClass
{
public:
    TestDataGenerator_constraint() : ServicesTestClass()
    {
        mongo::BSONObjBuilder builder;
        builder << "00080060" << "MR";
        this->add_constraint("Store", "root", builder.obj());
    }

    virtual ~TestDataGenerator_constraint()
    {
        // Nothing to do
    }
};

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    auto storegenerator = dopamine::services::StoreGenerator::New();
    BOOST_REQUIRE(storegenerator != NULL);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    auto storegenerator = dopamine::services::StoreGenerator::New();

    // Check default values for username
    BOOST_REQUIRE_EQUAL(storegenerator->get_username(), "");
    // Set username
    storegenerator->set_username("my_user");
    BOOST_REQUIRE_EQUAL(storegenerator->get_username(), "my_user");

    // Check default values for username
    BOOST_REQUIRE_EQUAL(storegenerator->get_peer_ae_title(), "");
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Initialize
 */
BOOST_FIXTURE_TEST_CASE(Initialize, ServicesTestClass)
{
    auto storegenerator = dopamine::services::StoreGenerator::New();

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    association.set_peer_ae_title("PEER");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID,
                {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"},
                dcmtkpp::VR::CS);
    dcmtkpp::message::CStoreRequest request(
            1, dcmtkpp::registry::MRImageStorage, SOP_INSTANCE_UID_01_01_01_01,
            dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = storegenerator->initialize(association, request);
    BOOST_CHECK_EQUAL(storegenerator->get_peer_ae_title(), "PEER");
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CStoreResponse::Pending);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Next
 */
BOOST_FIXTURE_TEST_CASE(Next, ServicesTestClass)
{
    auto storegenerator = dopamine::services::StoreGenerator::New();

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    association.set_peer_ae_title("PEER");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID,
                {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"},
                dcmtkpp::VR::CS);
    dcmtkpp::message::CStoreRequest request(
            1, dcmtkpp::registry::MRImageStorage, SOP_INSTANCE_UID_01_01_01_01,
            dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = storegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CStoreResponse::Pending);

    // unused function Next
    BOOST_REQUIRE(storegenerator->done());
    BOOST_REQUIRE_EQUAL(storegenerator->next(),
                        dcmtkpp::message::CStoreResponse::Success);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Insert Dataset
 */
BOOST_FIXTURE_TEST_CASE(InsertDataset, ServicesTestClass)
{
    mongo::unique_ptr<mongo::DBClientCursor> cursor =
        connection->get_connection().query(
            connection->get_db_name() + ".datasets",
            BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    association.set_peer_ae_title("PEER");

    auto storegenerator = dopamine::services::StoreGenerator::New();

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPClassUID,
                dcmtkpp::Element({dcmtkpp::registry::MRImageStorage},
                                 dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SOPInstanceUID,
                dcmtkpp::Element({SOP_INSTANCE_UID_03_01_01_01},
                                 dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::StudyInstanceUID,
                dcmtkpp::Element({STUDY_INSTANCE_UID_03_01},
                                 dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SeriesInstanceUID,
                dcmtkpp::Element({SERIES_INSTANCE_UID_03_01_01},
                                 dcmtkpp::VR::UI));

    dcmtkpp::message::CStoreRequest request(
            1, dcmtkpp::registry::MRImageStorage, SOP_INSTANCE_UID_01_01_01_01,
            dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = storegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CStoreResponse::Success);
    BOOST_REQUIRE(storegenerator->done());

    cursor = connection->get_connection().query(
                connection->get_db_name() + ".datasets",
                BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK_EQUAL(response.hasField("00080018"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000d"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000e"), true);

    BOOST_CHECK_EQUAL(cursor->more(), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Insert all attribute VR
 */
BOOST_FIXTURE_TEST_CASE(InsertCompleteDataset, ServicesTestClass)
{
    mongo::unique_ptr<mongo::DBClientCursor> cursor =
            connection->get_connection().query(
                connection->get_db_name() + ".datasets",
                BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    association.set_peer_ae_title("PEER");

    auto storegenerator = dopamine::services::StoreGenerator::New();

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::InstanceCreationDate,
                dcmtkpp::Element({"20150101"}, dcmtkpp::VR::DA));
    dataset.add(dcmtkpp::registry::InstanceCreationTime,
                dcmtkpp::Element({"101010"}, dcmtkpp::VR::TM));
    dataset.add(dcmtkpp::registry::SOPClassUID,
                dcmtkpp::Element({dcmtkpp::registry::MRImageStorage},
                                 dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SOPInstanceUID,
                dcmtkpp::Element({SOP_INSTANCE_UID_03_01_01_01},
                                 dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::AcquisitionDateTime,
                dcmtkpp::Element({"20150101101010.203"}, dcmtkpp::VR::DT));
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel,
                dcmtkpp::Element({"STUDY"}, dcmtkpp::VR::CS));
    dataset.add(dcmtkpp::registry::RetrieveAETitle,
                dcmtkpp::Element({"LOCAL"}, dcmtkpp::VR::AE));
    dataset.add(dcmtkpp::registry::Modality,
                dcmtkpp::Element({"MR"}, dcmtkpp::VR::CS));
    dataset.add(dcmtkpp::registry::Manufacturer,
                dcmtkpp::Element({"Manufacturer"}, dcmtkpp::VR::LO));
    dataset.add(dcmtkpp::registry::InstitutionAddress,
                dcmtkpp::Element({"value"}, dcmtkpp::VR::ST));
    dataset.add(dcmtkpp::registry::SimpleFrameList,
                dcmtkpp::Element({22}, dcmtkpp::VR::UL));
    dataset.add(dcmtkpp::registry::FailureReason,
                dcmtkpp::Element({42}, dcmtkpp::VR::US));
    dataset.add(dcmtkpp::registry::StageNumber,
                dcmtkpp::Element({12}, dcmtkpp::VR::IS));
    dataset.add(dcmtkpp::registry::RecommendedDisplayFrameRateInFloat,
                dcmtkpp::Element({42.5}, dcmtkpp::VR::FL));
    dataset.add(dcmtkpp::registry::PatientName,
                dcmtkpp::Element({"Name^Surname^Middle"}, dcmtkpp::VR::PN));

    dcmtkpp::DataSet sequence;
    sequence.add(dcmtkpp::registry::PatientID,
                 dcmtkpp::Element({"123"}, dcmtkpp::VR::LO));
    dataset.add(dcmtkpp::registry::OtherPatientIDsSequence,
                dcmtkpp::Element({sequence}, dcmtkpp::VR::SQ));

    dataset.add(dcmtkpp::registry::PatientAge,
                dcmtkpp::Element({"25Y"}, dcmtkpp::VR::AS));
    dataset.add(dcmtkpp::registry::PatientWeight,
                dcmtkpp::Element({11.11}, dcmtkpp::VR::DS));
    dataset.add(dcmtkpp::registry::EthnicGroup,
                dcmtkpp::Element({"value"}, dcmtkpp::VR::SH));
    dataset.add(dcmtkpp::registry::AdditionalPatientHistory,
                dcmtkpp::Element({"value"}, dcmtkpp::VR::LT));
    dataset.add(dcmtkpp::registry::ReferencePixelX0,
                dcmtkpp::Element({32}, dcmtkpp::VR::SL));
    dataset.add(dcmtkpp::registry::TagAngleSecondAxis,
                dcmtkpp::Element({32}, dcmtkpp::VR::SS));
    dataset.add(dcmtkpp::registry::StudyInstanceUID,
                dcmtkpp::Element({STUDY_INSTANCE_UID_03_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SeriesInstanceUID,
                dcmtkpp::Element({SERIES_INSTANCE_UID_03_01_01},
                                 dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::PixelDataProviderURL,
                dcmtkpp::Element({"value"}, dcmtkpp::VR::UR));
    dataset.add(dcmtkpp::registry::PupilSize,
                dcmtkpp::Element({42.5}, dcmtkpp::VR::FD));

    // Binary
    dcmtkpp::Value::Binary value = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
    dataset.add(dcmtkpp::registry::ICCProfile,
                dcmtkpp::Element(value, dcmtkpp::VR::OB));

    dcmtkpp::message::CStoreRequest request(
            1, dcmtkpp::registry::MRImageStorage, SOP_INSTANCE_UID_01_01_01_01,
            dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = storegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CStoreResponse::Success);
    BOOST_REQUIRE(storegenerator->done());

    cursor = connection->get_connection().query(
                connection->get_db_name() + ".datasets",
                BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK(response.hasField("00080012"));
    BOOST_CHECK(response.hasField("00080013"));
    BOOST_CHECK(response.hasField("00080016"));
    BOOST_CHECK(response.hasField("00080018"));
    BOOST_CHECK(response.hasField("0008002a"));
    BOOST_CHECK(response.hasField("00080052"));
    BOOST_CHECK(response.hasField("00080054"));
    BOOST_CHECK(response.hasField("00080060"));
    BOOST_CHECK(response.hasField("00080070"));
    BOOST_CHECK(response.hasField("00080081"));
    BOOST_CHECK(response.hasField("00081161"));
    BOOST_CHECK(response.hasField("00081197"));
    BOOST_CHECK(response.hasField("00082122"));
    BOOST_CHECK(response.hasField("00089459"));
    BOOST_CHECK(response.hasField("00100010"));
    BOOST_CHECK(response.hasField("00101002"));
    BOOST_CHECK(response.hasField("00101010"));
    BOOST_CHECK(response.hasField("00101030"));
    BOOST_CHECK(response.hasField("00102160"));
    BOOST_CHECK(response.hasField("001021b0"));
    BOOST_CHECK(response.hasField("00186020"));
    BOOST_CHECK(response.hasField("00189219"));
    BOOST_CHECK(response.hasField("0020000d"));
    BOOST_CHECK(response.hasField("0020000e"));
    BOOST_CHECK(response.hasField("00287fe0"));
    BOOST_CHECK(response.hasField("00460044"));
    BOOST_CHECK(response.hasField("Content")); // Dataset is stored in this field

    // The binary tags are not stored
    BOOST_CHECK(response.hasField("00282000") == false);

    BOOST_CHECK_EQUAL(cursor->more(), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Perform Store with user constraint
 */
BOOST_FIXTURE_TEST_CASE(Match_Constraint, TestDataGenerator_constraint)
{
    mongo::unique_ptr<mongo::DBClientCursor> cursor =
            this->connection->get_connection().query(
                this->connection->get_db_name() + ".datasets",
                BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("root");
    association.set_peer_ae_title("PEER");

    auto storegenerator = dopamine::services::StoreGenerator::New();

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPClassUID,
                dcmtkpp::Element({dcmtkpp::registry::MRImageStorage},
                                 dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SOPInstanceUID,
                dcmtkpp::Element({SOP_INSTANCE_UID_03_01_01_01},
                                 dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::Modality,
                dcmtkpp::Element({"MR"}, dcmtkpp::VR::CS));
    dataset.add(dcmtkpp::registry::StudyInstanceUID,
                dcmtkpp::Element({STUDY_INSTANCE_UID_03_01},
                                 dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SeriesInstanceUID,
                dcmtkpp::Element({SERIES_INSTANCE_UID_03_01_01},
                                 dcmtkpp::VR::UI));

    dcmtkpp::message::CStoreRequest request(
            1, dcmtkpp::registry::MRImageStorage, SOP_INSTANCE_UID_01_01_01_01,
            dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = storegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CStoreResponse::Success);
    BOOST_REQUIRE(storegenerator->done());

    cursor = this->connection->get_connection().query(
                this->connection->get_db_name() + ".datasets",
                BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK(response.hasField("00080018"));
    BOOST_CHECK(response.hasField("00080060"));
    BOOST_CHECK(response.hasField("0020000d"));
    BOOST_CHECK(response.hasField("0020000e"));
    BOOST_CHECK(response.hasField("Content")); // Dataset is stored in this field

    BOOST_CHECK_EQUAL(cursor->more(), false);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: User is not allow to perform Find
 *                  Status: RefusedNotAuthorized
 */
BOOST_FIXTURE_TEST_CASE(RefusedNotAuthorized, ServicesTestClass)
{
    auto storegenerator = dopamine::services::StoreGenerator::New();
    BOOST_REQUIRE_EQUAL(storegenerator->get_username(), "");

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("bad_user");
    association.set_peer_ae_title("PEER");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID,
                {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"},
                dcmtkpp::VR::CS);
    dcmtkpp::message::CStoreRequest request(
            1, dcmtkpp::registry::MRImageStorage, SOP_INSTANCE_UID_01_01_01_01,
            dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = storegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CStoreResponse::RefusedNotAuthorized);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Bad connection
 *                  Status: ProcessingFailure
 */
BOOST_AUTO_TEST_CASE(ProcessingFailure)
{
    auto storegenerator = dopamine::services::StoreGenerator::New();
    BOOST_REQUIRE_EQUAL(storegenerator->get_username(), "");

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    association.set_peer_ae_title("PEER");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID,
                {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"},
                dcmtkpp::VR::CS);
    dcmtkpp::message::CStoreRequest request(
            1, dcmtkpp::registry::MRImageStorage, SOP_INSTANCE_UID_01_01_01_01,
            dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = storegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CStoreResponse::ProcessingFailure);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Missing mandatory attribute SOPInstanceUID
 *                  Status: InvalidObjectInstance
 */
BOOST_FIXTURE_TEST_CASE(InvalidObjectInstance, ServicesTestClass)
{
    auto storegenerator = dopamine::services::StoreGenerator::New();
    BOOST_REQUIRE_EQUAL(storegenerator->get_username(), "");

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    association.set_peer_ae_title("PEER");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::PatientName, {"John"}, dcmtkpp::VR::PN);
    dcmtkpp::message::CStoreRequest request(
            1, dcmtkpp::registry::MRImageStorage, SOP_INSTANCE_UID_01_01_01_01,
            dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = storegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CStoreResponse::InvalidObjectInstance);
}
