/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleMoveSCP
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/DcmtkAssociation.h>
#include <dcmtkpp/message/CMoveResponse.h>
#include <dcmtkpp/Network.h>

#include "services/MoveGenerator.h"
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
    dcmtkpp::DcmtkAssociation association;

    movescp = new dopamine::services::MoveSCP(&network, &association);
    BOOST_REQUIRE(movescp != NULL);
    delete movescp;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    dopamine::services::MoveSCP movescp(NULL, NULL);

    // Check accessors of SCP base class
    BOOST_REQUIRE(movescp.get_generator() == NULL);
    movescp.set_generator(dopamine::services::MoveGenerator::New());
    BOOST_REQUIRE(movescp.get_generator() != NULL);
}
