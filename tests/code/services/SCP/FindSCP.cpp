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

#include "services/FindGenerator.h"
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
    delete findscp;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    dopamine::services::FindSCP findscp(NULL, NULL);

    // Check accessors of SCP base class
    BOOST_REQUIRE(findscp.get_generator() == NULL);
    findscp.set_generator(dopamine::services::FindGenerator::New());
    BOOST_REQUIRE(findscp.get_generator() != NULL);
}

