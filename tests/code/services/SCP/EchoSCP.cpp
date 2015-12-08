/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleEchoSCP
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/Association.h>
#include <dcmtkpp/message/CEchoResponse.h>
#include <dcmtkpp/Network.h>

#include "services/SCP/EchoSCP.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor/Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::services::EchoSCP * echoscp = new dopamine::services::EchoSCP();
    BOOST_REQUIRE(echoscp != NULL);
    delete echoscp; echoscp = NULL;

    dcmtkpp::Network network;
    dcmtkpp::Association association;

    echoscp = new dopamine::services::EchoSCP(&network, &association);
    BOOST_REQUIRE(echoscp != NULL);
    delete echoscp; echoscp = NULL;

    dopamine::services::EchoSCP::Callback callback =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CEchoRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CEchoResponse::Success; };

    echoscp = new dopamine::services::EchoSCP(&network, &association, callback);
    BOOST_REQUIRE(echoscp != NULL);
    delete echoscp;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    // Create EchoSCP with default Callback
    dopamine::services::EchoSCP::Callback callback =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CEchoRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CEchoResponse::Success; };

    dopamine::services::EchoSCP echoscp(NULL, NULL, callback);

    dcmtkpp::Association association;
    dcmtkpp::message::CEchoRequest request(1, "");

    // Retrieve default callback
    auto getcallback = echoscp.get_callback();
    BOOST_REQUIRE_EQUAL(getcallback(association, request, NULL),
                        dcmtkpp::message::CEchoResponse::Success);

    // Set a new callback
    dopamine::services::EchoSCP::Callback callback_toset =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CEchoRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CEchoResponse::Pending; };
    echoscp.set_callback(callback_toset);

    // Verify new callback is correctly set
    getcallback = echoscp.get_callback();
    BOOST_REQUIRE_EQUAL(getcallback(association, request, NULL),
                        dcmtkpp::message::CEchoResponse::Pending);
}
