/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleGetSCP
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/Association.h>
#include <dcmtkpp/message/CGetResponse.h>
#include <dcmtkpp/Network.h>

#include "services/SCP/GetSCP.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::services::GetSCP * getscp = new dopamine::services::GetSCP();
    BOOST_REQUIRE(getscp != NULL);
    delete getscp; getscp = NULL;

    dcmtkpp::Network network;
    dcmtkpp::Association association;

    getscp = new dopamine::services::GetSCP(&network, &association);
    BOOST_REQUIRE(getscp != NULL);
    delete getscp; getscp = NULL;

    dopamine::services::GetSCP::Callback callback =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CGetRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CGetResponse::Success; };

    getscp = new dopamine::services::GetSCP(&network, &association, callback);
    BOOST_REQUIRE(getscp != NULL);
    delete getscp;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    // Create GetSCP with default Callback
    dopamine::services::GetSCP::Callback callback =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CGetRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CGetResponse::Success; };

    dopamine::services::GetSCP getscp(NULL, NULL, callback);

    dcmtkpp::Association association;
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"123"}, dcmtkpp::VR::UI);
    dcmtkpp::message::CGetRequest request(1, "", 1, dataset);

    // Retrieve default callback
    auto getcallback = getscp.get_callback();
    BOOST_REQUIRE_EQUAL(getcallback(association, request, NULL),
                        dcmtkpp::message::CGetResponse::Success);

    // Set a new callback
    dopamine::services::GetSCP::Callback callback_toset =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CGetRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CGetResponse::Pending; };
    getscp.set_callback(callback_toset);

    // Verify new callback is correctly set
    getcallback = getscp.get_callback();
    BOOST_REQUIRE_EQUAL(getcallback(association, request, NULL),
                        dcmtkpp::message::CGetResponse::Pending);
}
