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

#include "services/GetGenerator.h"
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
    delete getscp;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    dopamine::services::GetSCP getscp(NULL, NULL);

    // Check accessors of SCP base class
    BOOST_REQUIRE(getscp.get_generator() == NULL);
    getscp.set_generator(dopamine::services::GetGenerator::New());
    BOOST_REQUIRE(getscp.get_generator() != NULL);
}
