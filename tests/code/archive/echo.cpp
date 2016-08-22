/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE echo
#include <boost/test/unit_test.hpp>

#include <odil/message/CEchoRequest.h>
#include <odil/message/Response.h>
#include <odil/registry.h>

#include "dopamine/archive/echo.h"

#include "fixtures/Authorization.h"

BOOST_FIXTURE_TEST_CASE(Echo, fixtures::Authorization)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username("echo");

    odil::message::CEchoRequest const request(
        1, odil::registry::VerificationSOPClass);

    auto const status = dopamine::archive::echo(
        this->connection, this->acl, parameters, request);
    BOOST_REQUIRE_EQUAL(status, odil::message::Response::Success);
}

BOOST_FIXTURE_TEST_CASE(EchoBadUser, fixtures::Authorization)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username("store");

    odil::message::CEchoRequest const request(
        1, odil::registry::VerificationSOPClass);

    auto const status = dopamine::archive::echo(
        this->connection, this->acl, parameters, request);
    BOOST_REQUIRE(odil::message::Response::is_failure(status));
}
