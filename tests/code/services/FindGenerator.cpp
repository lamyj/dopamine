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

    dcmtkpp::Association association;
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

    dcmtkpp::Association association;
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

    dcmtkpp::Association association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, {"IMAGE"}, dcmtkpp::VR::CS);
    dataset.add(dcmtkpp::registry::PersonName, {"John"}, dcmtkpp::VR::PN);
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

    dcmtkpp::Association association;
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

/******************************* TEST Error ************************************/
/**
 * Error test case: User is not allow to perform Find
 *                  Status: RefusedNotAuthorized
 */
BOOST_FIXTURE_TEST_CASE(RefusedNotAuthorized, ServicesTestClass)
{
    auto findgenerator = dopamine::services::FindGenerator::New();
    BOOST_REQUIRE_EQUAL(findgenerator->get_username(), "");

    dcmtkpp::Association association;
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

    dcmtkpp::Association association;
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

    dcmtkpp::Association association;
    association.set_user_identity_primary_field("");
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {SOP_INSTANCE_UID_01_01_01_01}, dcmtkpp::VR::UI);
    dcmtkpp::message::CFindRequest request(1, dcmtkpp::registry::MRImageStorage, dcmtkpp::message::Message::Priority::MEDIUM, dataset);
    auto status = findgenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CFindResponse::MissingAttribute);
}
