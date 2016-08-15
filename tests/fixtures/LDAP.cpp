/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "fixtures/LDAP.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace fixtures
{

LDAP
::LDAP()
{
    this->uri = LDAP::get_environment_variable("URI");
    this->bind_dn_template = LDAP::get_environment_variable("BIND_DN_TEMPLATE");
    this->username = LDAP::get_environment_variable("USERNAME");
    this->password = LDAP::get_environment_variable("PASSWORD");
}

LDAP
::~LDAP()
{
    // Nothing to do
}

std::string
LDAP
::get_environment_variable(std::string const & name)
{
    char const * const value = getenv(name.c_str());

    std::string result;
    if(value == nullptr)
    {
        throw std::runtime_error("Missing environment variable: "+name);
    }
    else
    {
        result = std::string(value);
    }

    return result;
}

} // namespace fixtures
