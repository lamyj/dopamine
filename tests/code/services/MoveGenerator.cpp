/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleMoveGenerator
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/message/CMoveRequest.h>
#include <dcmtkpp/message/CMoveResponse.h>

#include "services/MoveGenerator.h"
#include "ServicesTestClass.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    auto movegenerator = dopamine::services::MoveGenerator::New();
    BOOST_REQUIRE(movegenerator != NULL);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    auto movegenerator = dopamine::services::MoveGenerator::New();

    // Check default values for username
    BOOST_REQUIRE_EQUAL(movegenerator->get_username(), "");
    // Set username
    movegenerator->set_username("my_user");
    BOOST_REQUIRE_EQUAL(movegenerator->get_username(), "my_user");

    // Check default values for _query_retrieve_level
    BOOST_REQUIRE_EQUAL(movegenerator->get_query_retrieve_level(), "");
    // Set _query_retrieve_level
    movegenerator->set_query_retrieve_level("PATIENT");
    BOOST_REQUIRE_EQUAL(movegenerator->get_query_retrieve_level(), "PATIENT");

    // Check default values for _instance_count_tags
    BOOST_REQUIRE_EQUAL(movegenerator->get_instance_count_tags().size(), 0);

    // Check default values for _include_fields
    BOOST_REQUIRE_EQUAL(movegenerator->get_include_fields().size(), 0);
    // Set _include_fields
    movegenerator->set_include_fields({"00100010"});
    BOOST_REQUIRE_EQUAL(movegenerator->get_include_fields()[0], "00100010");

    // Check default values for _maximum_results
    BOOST_REQUIRE_EQUAL(movegenerator->get_maximum_results(), 0);
    // Set _include_fields
    movegenerator->set_maximum_results(1);
    BOOST_REQUIRE_EQUAL(movegenerator->get_maximum_results(), 1);

    // Check default values for _skipped_results
    BOOST_REQUIRE_EQUAL(movegenerator->get_skipped_results(), 0);
    // Set _skipped_results
    movegenerator->set_skipped_results(1);
    BOOST_REQUIRE_EQUAL(movegenerator->get_skipped_results(), 1);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Initialize
 */
BOOST_FIXTURE_TEST_CASE(Initialize, ServicesTestClass)
{
    auto movegenerator = dopamine::services::MoveGenerator::New();

    dcmtkpp::Association association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"123"}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dcmtkpp::message::CMoveRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, "DESTINATION", dataset);
    auto status = movegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CMoveResponse::Pending);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Next
 */
BOOST_FIXTURE_TEST_CASE(Next, ServicesTestClass)
{
    auto movegenerator = dopamine::services::MoveGenerator::New();

    dcmtkpp::Association association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dcmtkpp::message::CMoveRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, "DESTINATION", dataset);
    auto status = movegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CMoveResponse::Pending);

    // Find one result
    BOOST_REQUIRE(!movegenerator->done());
    BOOST_REQUIRE_EQUAL(movegenerator->next(),
                        dcmtkpp::message::CMoveResponse::Pending);
    auto data_set = movegenerator->get();
    BOOST_REQUIRE(!data_set.second.empty());
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPInstanceUID));
    BOOST_REQUIRE_EQUAL(data_set.second.as_string(dcmtkpp::registry::SOPInstanceUID)[0], SOP_INSTANCE_UID_01_01_01_01);

    // No more result
    BOOST_REQUIRE(movegenerator->done());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: No result
 */
BOOST_FIXTURE_TEST_CASE(NoDataset, ServicesTestClass)
{
    auto movegenerator = dopamine::services::MoveGenerator::New();

    dcmtkpp::Association association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dataset.add(dcmtkpp::registry::PatientName, {"John"}, dcmtkpp::VR::PN);
    dcmtkpp::message::CMoveRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, "DESTINATION", dataset);
    auto status = movegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CMoveResponse::Pending);

    // Find no result
    BOOST_REQUIRE(movegenerator->done());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Get include fields
 */
BOOST_FIXTURE_TEST_CASE(GetIncludeFields, ServicesTestClass)
{
    auto movegenerator = dopamine::services::MoveGenerator::New();

    dcmtkpp::Association association;
    association.set_user_identity_primary_field("");

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    movegenerator->set_include_fields({"00100020"});
    dcmtkpp::message::CMoveRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, "DESTINATION", dataset);
    auto status = movegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CMoveResponse::Pending);

    // Find one result
    BOOST_REQUIRE(!movegenerator->done());
    BOOST_REQUIRE_EQUAL(movegenerator->next(),
                        dcmtkpp::message::CMoveResponse::Pending);
    auto data_set = movegenerator->get();
    BOOST_REQUIRE(!data_set.second.empty());
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPInstanceUID));
    BOOST_REQUIRE_EQUAL(data_set.second.as_string(dcmtkpp::registry::SOPInstanceUID)[0], SOP_INSTANCE_UID_01_01_01_01);
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::PatientID));
    BOOST_REQUIRE_EQUAL(data_set.second.as_string(dcmtkpp::registry::PatientID)[0], "dopamine_test_01");

    // No more result
    BOOST_REQUIRE(movegenerator->done());
}

/******************************* TEST Error ************************************/
/**
 * Error test case: User is not allow to perform Find
 *                  Status: RefusedNotAuthorized
 */
BOOST_FIXTURE_TEST_CASE(RefusedNotAuthorized, ServicesTestClass)
{
    auto movegenerator = dopamine::services::MoveGenerator::New();
    BOOST_REQUIRE_EQUAL(movegenerator->get_username(), "");

    dcmtkpp::Association association;
    association.set_user_identity_primary_field("bad_user");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dcmtkpp::message::CMoveRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, "DESTINATION", dataset);
    auto status = movegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CMoveResponse::RefusedNotAuthorized);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Bad connection
 *                  Status: ProcessingFailure
 */
BOOST_AUTO_TEST_CASE(ProcessingFailure)
{
    auto movegenerator = dopamine::services::MoveGenerator::New();
    BOOST_REQUIRE_EQUAL(movegenerator->get_username(), "");

    dcmtkpp::Association association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dcmtkpp::message::CMoveRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, "DESTINATION", dataset);
    auto status = movegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CMoveResponse::ProcessingFailure);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Missing mandatory attribute Query Retrieve Level
 *                  Status: MissingAttribute
 */
BOOST_FIXTURE_TEST_CASE(MissingAttribute, ServicesTestClass)
{
    auto movegenerator = dopamine::services::MoveGenerator::New();
    BOOST_REQUIRE_EQUAL(movegenerator->get_username(), "");

    dcmtkpp::Association association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dcmtkpp::message::CMoveRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, "DESTINATION", dataset);
    auto status = movegenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CMoveResponse::MissingAttribute);
}
