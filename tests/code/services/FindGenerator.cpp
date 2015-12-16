/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleFindGenerator
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/message/CFindRequest.h>
#include <dcmtkpp/message/CFindResponse.h>

#include "services/FindGenerator.h"
#include "ServicesTestClass.h"

class FindGeneratorTest : public ServicesTestClass
{
public:
    FindGeneratorTest() : ServicesTestClass(), _sop_instance_uid_for_find("1.2.3.4.5.6")
    {
        // Create a new dataset
        _dataset.add(dcmtkpp::registry::InstanceCreationDate, {"20151201"}, dcmtkpp::VR::DA);
        _dataset.add(dcmtkpp::registry::InstanceCreationTime, {"090909"}, dcmtkpp::VR::TM);
        _dataset.add(dcmtkpp::registry::SOPClassUID, {dcmtkpp::registry::PositronEmissionTomographyImageStorage}, dcmtkpp::VR::UI);
        _dataset.add(dcmtkpp::registry::SOPInstanceUID, {this->_sop_instance_uid_for_find}, dcmtkpp::VR::UI);
        _dataset.add(dcmtkpp::registry::AcquisitionDateTime, {"20150101101010.202"}, dcmtkpp::VR::DT);
        _dataset.add(dcmtkpp::registry::RetrieveAETitle, {"LOCAL"}, dcmtkpp::VR::AE);
        _dataset.add(dcmtkpp::registry::Modality, {"MyModality"}, dcmtkpp::VR::CS);
        _dataset.add(dcmtkpp::registry::Manufacturer, {"Manufacturer"}, dcmtkpp::VR::LO);
        _dataset.add(dcmtkpp::registry::InstitutionAddress, {"value"}, dcmtkpp::VR::ST);
        _dataset.add(dcmtkpp::registry::SimpleFrameList, {22}, dcmtkpp::VR::UL);
        _dataset.add(dcmtkpp::registry::FailureReason, {42}, dcmtkpp::VR::US);
        _dataset.add(dcmtkpp::registry::StageNumber, {12}, dcmtkpp::VR::IS);
        _dataset.add(dcmtkpp::registry::RecommendedDisplayFrameRateInFloat, {42.5}, dcmtkpp::VR::FL);
        _dataset.add(dcmtkpp::registry::PatientName, {"Name^Surname^Middle"}, dcmtkpp::VR::PN);
        _dataset.add(dcmtkpp::registry::PatientAge, {"89Y"}, dcmtkpp::VR::AS);
        _dataset.add(dcmtkpp::registry::PatientWeight, {11.11}, dcmtkpp::VR::DS);
        _dataset.add(dcmtkpp::registry::EthnicGroup, {"value"}, dcmtkpp::VR::SH);
        _dataset.add(dcmtkpp::registry::AdditionalPatientHistory, {"value"}, dcmtkpp::VR::LT);
        _dataset.add(dcmtkpp::registry::ReferencePixelX0, {32}, dcmtkpp::VR::SL);
        _dataset.add(dcmtkpp::registry::TagAngleSecondAxis, {32}, dcmtkpp::VR::SS);
        _dataset.add(dcmtkpp::registry::ICCProfile, dcmtkpp::Value::Binary({0x1, 0x2, 0x3, 0x4, 0x5}), dcmtkpp::VR::OB);
        _dataset.add(dcmtkpp::registry::PixelDataProviderURL, {"value"}, dcmtkpp::VR::UT);
        _dataset.add(dcmtkpp::registry::PupilSize, {42.5}, dcmtkpp::VR::FD);

        insert_dataset(_dataset);
    }

    virtual ~FindGeneratorTest()
    {
        // Nothing to do.
    }

    void find(dcmtkpp::Tag const & tag, dcmtkpp::Element const & element)
    {
        auto findgenerator = dopamine::services::FindGenerator::New();
        findgenerator->set_include_fields({"00080018"}); // get the SOP Instance UID

        dcmtkpp::DcmtkAssociation association;
        association.set_user_identity_primary_field("");

        dcmtkpp::DataSet dataset;
        dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"STUDY"}, dcmtkpp::VR::CS);
        dataset.add(tag, element);

        dcmtkpp::message::CFindRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, dataset);
        auto status = findgenerator->initialize(association, request);
        BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CFindResponse::Pending);

        // Find one result
        BOOST_REQUIRE(!findgenerator->done());
        BOOST_REQUIRE_EQUAL(findgenerator->next(),
                            dcmtkpp::message::CFindResponse::Pending);
        auto data_set = findgenerator->get();
        BOOST_REQUIRE(!data_set.second.empty());
        BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPInstanceUID));
        BOOST_REQUIRE_EQUAL(data_set.second.as_string(dcmtkpp::registry::SOPInstanceUID)[0], this->_sop_instance_uid_for_find);

        // No more result
        BOOST_REQUIRE(findgenerator->done());
    }

    dcmtkpp::DataSet _dataset;

private:
    std::string _sop_instance_uid_for_find;

};

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    auto findgenerator = dopamine::services::FindGenerator::New();
    BOOST_REQUIRE(findgenerator != NULL);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    auto findgenerator = dopamine::services::FindGenerator::New();

    // Check default values for username
    BOOST_REQUIRE_EQUAL(findgenerator->get_username(), "");
    // Set username
    findgenerator->set_username("my_user");
    BOOST_REQUIRE_EQUAL(findgenerator->get_username(), "my_user");

    // Check default values for _query_retrieve_level
    BOOST_REQUIRE_EQUAL(findgenerator->get_query_retrieve_level(), "");
    // Set _query_retrieve_level
    findgenerator->set_query_retrieve_level("PATIENT");
    BOOST_REQUIRE_EQUAL(findgenerator->get_query_retrieve_level(), "PATIENT");

    // Check default values for _instance_count_tags
    BOOST_REQUIRE_EQUAL(findgenerator->get_instance_count_tags().size(), 0);

    // Check default values for _convert_modalities_in_study
    BOOST_REQUIRE_EQUAL(findgenerator->get_convert_modalities_in_study(), false);

    // Check default values for _include_fields
    BOOST_REQUIRE_EQUAL(findgenerator->get_include_fields().size(), 0);
    // Set _include_fields
    findgenerator->set_include_fields({"00100010"});
    BOOST_REQUIRE_EQUAL(findgenerator->get_include_fields()[0], "00100010");

    // Check default values for _maximum_results
    BOOST_REQUIRE_EQUAL(findgenerator->get_maximum_results(), 0);
    // Set _include_fields
    findgenerator->set_maximum_results(1);
    BOOST_REQUIRE_EQUAL(findgenerator->get_maximum_results(), 1);

    // Check default values for _skipped_results
    BOOST_REQUIRE_EQUAL(findgenerator->get_skipped_results(), 0);
    // Set _skipped_results
    findgenerator->set_skipped_results(1);
    BOOST_REQUIRE_EQUAL(findgenerator->get_skipped_results(), 1);

    // Check default values for _fuzzy_matching
    BOOST_REQUIRE_EQUAL(findgenerator->get_fuzzy_matching(), false);
    // Set _fuzzy_matching
    findgenerator->set_fuzzy_matching(true);
    BOOST_REQUIRE_EQUAL(findgenerator->get_fuzzy_matching(), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Initialize
 */
BOOST_FIXTURE_TEST_CASE(Initialize, ServicesTestClass)
{
    auto findgenerator = dopamine::services::FindGenerator::New();

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"123"}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dcmtkpp::message::CFindRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = findgenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CFindResponse::Pending);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Next
 */
BOOST_FIXTURE_TEST_CASE(Next, ServicesTestClass)
{
    auto findgenerator = dopamine::services::FindGenerator::New();

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dcmtkpp::message::CFindRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = findgenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CFindResponse::Pending);

    // Find one result
    BOOST_REQUIRE(!findgenerator->done());
    BOOST_REQUIRE_EQUAL(findgenerator->next(),
                        dcmtkpp::message::CFindResponse::Pending);
    auto data_set = findgenerator->get();
    BOOST_REQUIRE(!data_set.second.empty());
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPInstanceUID));
    BOOST_REQUIRE_EQUAL(data_set.second.as_string(dcmtkpp::registry::SOPInstanceUID)[0], SOP_INSTANCE_UID_01_01_01_01);

    // No more result
    BOOST_REQUIRE(findgenerator->done());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: No result
 */
BOOST_FIXTURE_TEST_CASE(NoDataset, ServicesTestClass)
{
    auto findgenerator = dopamine::services::FindGenerator::New();

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dataset.add(dcmtkpp::registry::PatientName, {"John"}, dcmtkpp::VR::PN);
    dcmtkpp::message::CFindRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = findgenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CFindResponse::Pending);

    // Find no result
    BOOST_REQUIRE(findgenerator->done());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Get include fields
 */
BOOST_FIXTURE_TEST_CASE(GetIncludeFields, ServicesTestClass)
{
    auto findgenerator = dopamine::services::FindGenerator::New();

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    findgenerator->set_include_fields({"00100020"});
    dcmtkpp::message::CFindRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = findgenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CFindResponse::Pending);

    // Find one result
    BOOST_REQUIRE(!findgenerator->done());
    BOOST_REQUIRE_EQUAL(findgenerator->next(),
                        dcmtkpp::message::CFindResponse::Pending);
    auto data_set = findgenerator->get();
    BOOST_REQUIRE(!data_set.second.empty());
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPInstanceUID));
    BOOST_REQUIRE_EQUAL(data_set.second.as_string(dcmtkpp::registry::SOPInstanceUID)[0], SOP_INSTANCE_UID_01_01_01_01);
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::PatientID));
    BOOST_REQUIRE_EQUAL(data_set.second.as_string(dcmtkpp::registry::PatientID)[0], "dopamine_test_01");

    // No more result
    BOOST_REQUIRE(findgenerator->done());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Request with all attributes VR
 */
BOOST_FIXTURE_TEST_CASE(RequestAllAttributes, ServicesTestClass)
{
    auto findgenerator = dopamine::services::FindGenerator::New();

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::InstanceCreationDate, {"20150101"}, dcmtkpp::VR::DA);
    dataset.add(dcmtkpp::registry::InstanceCreationTime, {"101010"}, dcmtkpp::VR::TM);
    dataset.add(dcmtkpp::registry::SOPClassUID, {dcmtkpp::registry::MRImageStorage}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::AcquisitionDateTime, {"20150101101010.203"}, dcmtkpp::VR::DT);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"STUDY"}, dcmtkpp::VR::CS);
    dataset.add(dcmtkpp::registry::RetrieveAETitle, {"LOCAL"}, dcmtkpp::VR::AE);
    dataset.add(dcmtkpp::registry::Modality, {"MR"}, dcmtkpp::VR::CS);
    dataset.add(dcmtkpp::registry::Manufacturer, {"Manufacturer"}, dcmtkpp::VR::LO);
    dataset.add(dcmtkpp::registry::InstitutionAddress, {"value"}, dcmtkpp::VR::ST);
    dataset.add(dcmtkpp::registry::SimpleFrameList, {22}, dcmtkpp::VR::UL);
    dataset.add(dcmtkpp::registry::FailureReason, {42}, dcmtkpp::VR::US);
    dataset.add(dcmtkpp::registry::StageNumber, {12}, dcmtkpp::VR::IS);
    dataset.add(dcmtkpp::registry::RecommendedDisplayFrameRateInFloat, {42.5}, dcmtkpp::VR::FL);
    dataset.add(dcmtkpp::registry::PatientName, {"Name^Surname^Middle"}, dcmtkpp::VR::PN);
    dataset.add(dcmtkpp::registry::PatientAge, {"25Y"}, dcmtkpp::VR::AS);
    dataset.add(dcmtkpp::registry::PatientWeight, {11.11}, dcmtkpp::VR::DS);
    dataset.add(dcmtkpp::registry::EthnicGroup, {"value"}, dcmtkpp::VR::SH);
    dataset.add(dcmtkpp::registry::AdditionalPatientHistory, {"value"}, dcmtkpp::VR::LT);
    dataset.add(dcmtkpp::registry::ReferencePixelX0, {32}, dcmtkpp::VR::SL);
    dataset.add(dcmtkpp::registry::TagAngleSecondAxis, {32}, dcmtkpp::VR::SS);
    dataset.add(dcmtkpp::registry::ICCProfile, dcmtkpp::Value::Binary({0x1, 0x2, 0x3, 0x4, 0x5}), dcmtkpp::VR::OB);
    dataset.add(dcmtkpp::registry::PixelDataProviderURL, {"value"}, dcmtkpp::VR::UT);
    dataset.add(dcmtkpp::registry::PupilSize, {42.5}, dcmtkpp::VR::FD);

    dcmtkpp::message::CFindRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = findgenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CFindResponse::Pending);

    // Find no result
    BOOST_REQUIRE(findgenerator->done());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Request Match for each VR
 */
BOOST_FIXTURE_TEST_CASE(RequestEachVR, FindGeneratorTest)
{
    for (auto item : this->_dataset)
    {
        if (item.second.vr == dcmtkpp::VR::OB ||
            item.second.vr == dcmtkpp::VR::OF ||
            item.second.vr == dcmtkpp::VR::OW ||
            item.second.vr == dcmtkpp::VR::UN)
        {
            // don't watch for binary VR
            continue;
        }
        find(item.first, item.second);
    }
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Request Match Regex
 */
BOOST_FIXTURE_TEST_CASE(RequestRegEx, FindGeneratorTest)
{
    find(dcmtkpp::registry::PatientName, dcmtkpp::Element({"N?me*"}, dcmtkpp::VR::PN));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Request Match Range
 */
BOOST_FIXTURE_TEST_CASE(RequestRange, FindGeneratorTest)
{
    find(dcmtkpp::registry::InstanceCreationDate, dcmtkpp::Element({"20151105-20151220"}, dcmtkpp::VR::DA));
}

/******************************* TEST Error ************************************/
/**
 * Error test case: User is not allow to perform Find
 *                  Status: RefusedNotAuthorized
 */
BOOST_FIXTURE_TEST_CASE(RefusedNotAuthorized, ServicesTestClass)
{
    auto findgenerator = dopamine::services::FindGenerator::New();
    BOOST_REQUIRE_EQUAL(findgenerator->get_username(), "");

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("bad_user");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dcmtkpp::message::CFindRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = findgenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CFindResponse::RefusedNotAuthorized);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Bad connection
 *                  Status: ProcessingFailure
 */
BOOST_AUTO_TEST_CASE(ProcessingFailure)
{
    auto findgenerator = dopamine::services::FindGenerator::New();
    BOOST_REQUIRE_EQUAL(findgenerator->get_username(), "");

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dcmtkpp::message::CFindRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = findgenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CFindResponse::ProcessingFailure);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Missing mandatory attribute Query Retrieve Level
 *                  Status: MissingAttribute
 */
BOOST_FIXTURE_TEST_CASE(MissingAttribute, ServicesTestClass)
{
    auto findgenerator = dopamine::services::FindGenerator::New();
    BOOST_REQUIRE_EQUAL(findgenerator->get_username(), "");

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dcmtkpp::message::CFindRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = findgenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CFindResponse::MissingAttribute);
}
