/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleStoreSCP
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/Association.h>
#include <dcmtkpp/message/CStoreResponse.h>
#include <dcmtkpp/Network.h>

#include "services/SCP/StoreSCP.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::services::StoreSCP * storescp = new dopamine::services::StoreSCP();
    BOOST_REQUIRE(storescp != NULL);
    delete storescp; storescp = NULL;

    dcmtkpp::Network network;
    dcmtkpp::Association association;

    storescp = new dopamine::services::StoreSCP(&network, &association);
    BOOST_REQUIRE(storescp != NULL);
    delete storescp; storescp = NULL;

    dopamine::services::StoreSCP::Callback callback =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CStoreRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CStoreResponse::Success; };

    storescp = new dopamine::services::StoreSCP(&network, &association,
                                                callback);
    BOOST_REQUIRE(storescp != NULL);
    delete storescp;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    // Create StoreSCP with default Callback
    dopamine::services::StoreSCP::Callback callback =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CStoreRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CStoreResponse::Success; };

    dopamine::services::StoreSCP storescp(NULL, NULL, callback);

    dcmtkpp::Association association;
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"123"}, dcmtkpp::VR::UI);
    dcmtkpp::message::CStoreRequest request(1, "", "", 1, dataset);

    // Retrieve default callback
    auto getcallback = storescp.get_callback();
    BOOST_REQUIRE_EQUAL(getcallback(association, request, NULL),
                        dcmtkpp::message::CStoreResponse::Success);

    // Set a new callback
    dopamine::services::StoreSCP::Callback callback_toset =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CStoreRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CStoreResponse::Pending; };
    storescp.set_callback(callback_toset);

    // Verify new callback is correctly set
    getcallback = storescp.get_callback();
    BOOST_REQUIRE_EQUAL(getcallback(association, request, NULL),
                        dcmtkpp::message::CStoreResponse::Pending);
}

