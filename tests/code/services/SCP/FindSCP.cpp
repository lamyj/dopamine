/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleFindSCP
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/Association.h>
#include <dcmtkpp/message/CFindResponse.h>
#include <dcmtkpp/Network.h>

#include "services/SCP/FindSCP.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor/Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::services::FindSCP * findscp = new dopamine::services::FindSCP();
    BOOST_REQUIRE(findscp != NULL);
    delete findscp; findscp = NULL;

    dcmtkpp::Network network;
    dcmtkpp::Association association;

    findscp = new dopamine::services::FindSCP(&network, &association);
    BOOST_REQUIRE(findscp != NULL);
    delete findscp; findscp = NULL;

    dopamine::services::FindSCP::Callback callback =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CFindRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CFindResponse::Success; };

    findscp = new dopamine::services::FindSCP(&network, &association, callback);
    BOOST_REQUIRE(findscp != NULL);
    delete findscp;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    // Create FindSCP with default Callback
    dopamine::services::FindSCP::Callback callback =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CFindRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CFindResponse::Success; };

    dopamine::services::FindSCP findscp(NULL, NULL, callback);

    dcmtkpp::Association association;
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"123"}, dcmtkpp::VR::UI);
    dcmtkpp::message::CFindRequest request(1, "", 1, dataset);

    // Retrieve default callback
    auto getcallback = findscp.get_callback();
    BOOST_REQUIRE_EQUAL(getcallback(association, request, NULL),
                        dcmtkpp::message::CFindResponse::Success);

    // Set a new callback
    dopamine::services::FindSCP::Callback callback_toset =
            [](dcmtkpp::Association const & association,
               dcmtkpp::message::CFindRequest const & request,
               dopamine::services::Generator::Pointer generator)
        { return dcmtkpp::message::CFindResponse::Pending; };
    findscp.set_callback(callback_toset);

    // Verify new callback is correctly set
    getcallback = findscp.get_callback();
    BOOST_REQUIRE_EQUAL(getcallback(association, request, NULL),
                        dcmtkpp::message::CFindResponse::Pending);
}

