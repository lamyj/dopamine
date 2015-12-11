/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleSCPDispatcher
#include <boost/test/unit_test.hpp>

#include "core/SCPDispatcher.h"
#include "services/SCP/EchoSCP.h"
#include "services/SCP/FindSCP.h"
#include "services/SCP/GetSCP.h"
#include "services/SCP/MoveSCP.h"
#include "services/SCP/StoreSCP.h"

template<typename TSCP>
void test_set_scp(dcmtkpp::Value::Integer command,
                  TSCP const & scp)
{
    dopamine::SCPDispatcher dispatcher;
    BOOST_REQUIRE(!dispatcher.has_scp(command));
    dispatcher.set_scp(command, scp);
    BOOST_REQUIRE(dispatcher.has_scp(command));

    auto & getscp = dispatcher.get_scp(command);
    BOOST_REQUIRE(getscp.get_association() == NULL);
    dopamine::services::SCP const & getconstscp = dispatcher.get_scp(command);
    BOOST_REQUIRE(getconstscp.get_association() == NULL);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::SCPDispatcher * dispatcher = new dopamine::SCPDispatcher();
    BOOST_REQUIRE(dispatcher != NULL);
    delete dispatcher;

    dispatcher = new dopamine::SCPDispatcher(NULL, NULL);
    BOOST_REQUIRE(dispatcher != NULL);
    delete dispatcher;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    test_set_scp(dcmtkpp::message::Message::Command::C_ECHO_RQ,
                 dopamine::services::EchoSCP());
    test_set_scp(dcmtkpp::message::Message::Command::C_FIND_RQ,
                 dopamine::services::FindSCP());
    test_set_scp(dcmtkpp::message::Message::Command::C_GET_RQ,
                 dopamine::services::GetSCP());
    test_set_scp(dcmtkpp::message::Message::Command::C_MOVE_RQ,
                 dopamine::services::MoveSCP());
    test_set_scp(dcmtkpp::message::Message::Command::C_STORE_RQ,
                 dopamine::services::StoreSCP());
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Get unknown SCP
 */
BOOST_AUTO_TEST_CASE(ProcessingFailure)
{
    dopamine::SCPDispatcher dispatcher;
    BOOST_REQUIRE_THROW(
        dispatcher.get_scp(dcmtkpp::message::Message::Command::C_CANCEL_RQ),
        dcmtkpp::Exception);
}
