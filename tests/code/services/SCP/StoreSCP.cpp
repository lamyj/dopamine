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
#include "services/StoreGenerator.h"

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
    delete storescp;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    dopamine::services::StoreSCP storescp(NULL, NULL);

    // Check accessors of SCP base class
    BOOST_REQUIRE(storescp.get_generator() == NULL);
    storescp.set_generator(dopamine::services::StoreGenerator::New());
    BOOST_REQUIRE(storescp.get_generator() != NULL);
}

