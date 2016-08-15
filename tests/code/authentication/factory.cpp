/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE factory
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <map>
#include <memory>
#include <string>

#include <odil/AssociationParameters.h>

#include "dopamine/authentication/factory.h"
#include "dopamine/authentication/AuthenticatorCSV.h"
#include "dopamine/authentication/AuthenticatorLDAP.h"
#include "dopamine/authentication/AuthenticatorNone.h"

#include "fixtures/CSV.h"
#include "fixtures/LDAP.h"

BOOST_AUTO_TEST_CASE(None)
{
    std::map<std::string, std::string> authentication{ { "type", "None" } };
    auto const authenticator = dopamine::authentication::factory(authentication);
    auto const concrete_authenticator =
        std::dynamic_pointer_cast<dopamine::authentication::AuthenticatorNone>(
            authenticator);
    BOOST_REQUIRE(concrete_authenticator);
}

BOOST_FIXTURE_TEST_CASE(CSV, fixtures::CSV)
{
    std::map<std::string, std::string> authentication{
        { "type", "CSV" },
        { "filepath", filename }};
    auto const authenticator = dopamine::authentication::factory(authentication);
    auto const concrete_authenticator =
        std::dynamic_pointer_cast<dopamine::authentication::AuthenticatorCSV>(
            authenticator);
    BOOST_REQUIRE(concrete_authenticator);

    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password("user2", "password2");
    BOOST_REQUIRE((*authenticator)(parameters));
}

BOOST_FIXTURE_TEST_CASE(LDAP, fixtures::LDAP)
{
    std::map<std::string, std::string> authentication{
        { "type", "LDAP" },
        { "uri", uri },
        { "bind_dn_template", bind_dn_template }};
    auto const authenticator = dopamine::authentication::factory(authentication);
    auto const concrete_authenticator =
        std::dynamic_pointer_cast<dopamine::authentication::AuthenticatorLDAP>(
            authenticator);
    BOOST_REQUIRE(concrete_authenticator);

    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username_and_password(username, password);
    BOOST_REQUIRE((*authenticator)(parameters));
}
