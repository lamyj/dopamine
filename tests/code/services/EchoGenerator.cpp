/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleEchoGenerator
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/message/CEchoRequest.h>
#include <dcmtkpp/message/CEchoResponse.h>

#include "services/EchoGenerator.h"
#include "ServicesTestClass.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    auto echogenerator = dopamine::services::EchoGenerator::New();
    BOOST_REQUIRE(echogenerator != NULL);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    // Check default values for username
    auto echogenerator = dopamine::services::EchoGenerator::New();
    BOOST_REQUIRE_EQUAL(echogenerator->get_username(), "");

    // Set username
    echogenerator->set_username("my_user");
    BOOST_REQUIRE_EQUAL(echogenerator->get_username(), "my_user");
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Initialize
 */
BOOST_FIXTURE_TEST_CASE(Initialize, ServicesTestClass)
{
    auto echogenerator = dopamine::services::EchoGenerator::New();
    BOOST_REQUIRE_EQUAL(echogenerator->get_username(), "");

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    dcmtkpp::message::CEchoRequest request(1, "");
    auto status = echogenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CEchoResponse::Pending);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Next
 */
BOOST_FIXTURE_TEST_CASE(Next, ServicesTestClass)
{
    auto echogenerator = dopamine::services::EchoGenerator::New();
    BOOST_REQUIRE_EQUAL(echogenerator->get_username(), "");

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    dcmtkpp::message::CEchoRequest request(1, "");
    auto status = echogenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status, dcmtkpp::message::CEchoResponse::Pending);

    BOOST_REQUIRE(echogenerator->done());
    BOOST_REQUIRE_EQUAL(echogenerator->next(),
                        dcmtkpp::message::CEchoResponse::Success);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: User is not allow to perform Echo
 *                  Status: RefusedNotAuthorized
 */
BOOST_FIXTURE_TEST_CASE(RefusedNotAuthorized, ServicesTestClass)
{
    auto echogenerator = dopamine::services::EchoGenerator::New();
    BOOST_REQUIRE_EQUAL(echogenerator->get_username(), "");

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("bad_user");
    dcmtkpp::message::CEchoRequest request(1, "");
    auto status = echogenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CEchoResponse::RefusedNotAuthorized);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Bad connection
 *                  Status: ProcessingFailure
 */
BOOST_AUTO_TEST_CASE(ProcessingFailure)
{
    auto echogenerator = dopamine::services::EchoGenerator::New();
    BOOST_REQUIRE_EQUAL(echogenerator->get_username(), "");

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_primary_field("");
    dcmtkpp::message::CEchoRequest request(1, "");
    auto status = echogenerator->initialize(association, request);
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::CEchoResponse::ProcessingFailure);
}
