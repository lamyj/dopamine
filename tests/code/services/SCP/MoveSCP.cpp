/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleMoveSCP
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/Association.h>
#include <dcmtkpp/message/CMoveResponse.h>
#include <dcmtkpp/Network.h>

#include "services/SCP/MoveSCP.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::services::MoveSCP * movescp = new dopamine::services::MoveSCP();
    BOOST_REQUIRE(movescp != NULL);
    delete movescp; movescp = NULL;

    dcmtkpp::Network network;
    dcmtkpp::Association association;

    movescp = new dopamine::services::MoveSCP(&network, &association);
    BOOST_REQUIRE(movescp != NULL);
    delete movescp; movescp = NULL;

    dopamine::services::MoveSCP::Callback callback =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CMoveRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CMoveResponse::Success; };

    movescp = new dopamine::services::MoveSCP(&network, &association, callback);
    BOOST_REQUIRE(movescp != NULL);
    delete movescp;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    // Create MoveSCP with default Callback
    dopamine::services::MoveSCP::Callback callback =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CMoveRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CMoveResponse::Success; };

    dopamine::services::MoveSCP movescp(NULL, NULL, callback);

    dcmtkpp::Association association;
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"123"}, dcmtkpp::VR::UI);
    dcmtkpp::message::CMoveRequest request(1, "", 1, "", dataset);

    // Retrieve default callback
    auto getcallback = movescp.get_callback();
    BOOST_REQUIRE_EQUAL(getcallback(association, request, NULL),
                        dcmtkpp::message::CMoveResponse::Success);

    // Set a new callback
    dopamine::services::MoveSCP::Callback callback_toset =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CMoveRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CMoveResponse::Pending; };
    movescp.set_callback(callback_toset);

    // Verify new callback is correctly set
    getcallback = movescp.get_callback();
    BOOST_REQUIRE_EQUAL(getcallback(association, request, NULL),
                        dcmtkpp::message::CMoveResponse::Pending);
}
