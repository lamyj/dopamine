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

#include "services/EchoGenerator.h"
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
    delete echoscp;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    dopamine::services::EchoSCP echoscp(NULL, NULL);

    // Check accessors of SCP base class
    BOOST_REQUIRE(echoscp.get_generator() == NULL);
    echoscp.set_generator(dopamine::services::EchoGenerator::New());
    BOOST_REQUIRE(echoscp.get_generator() != NULL);
}
