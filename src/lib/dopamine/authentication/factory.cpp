/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/authentication/factory.h"

#include <map>
#include <memory>
#include <string>

#include "dopamine/authentication/AuthenticatorBase.h"
#include "dopamine/authentication/AuthenticatorCSV.h"
#include "dopamine/authentication/AuthenticatorLDAP.h"
#include "dopamine/authentication/AuthenticatorNone.h"
#include "dopamine/Exception.h"

namespace dopamine
{

namespace authentication
{

std::shared_ptr<AuthenticatorBase> factory(
    std::map<std::string, std::string> const & properties)
{
    auto const & type = properties.at("type");
    if(type == "None")
    {
        return std::make_shared<AuthenticatorNone>();
    }
    else if(type == "CSV")
    {
        return std::make_shared<AuthenticatorCSV>(properties.at("filepath"));
    }
    else if(type == "LDAP")
    {
        return std::make_shared<AuthenticatorLDAP>(
            properties.at("uri"), properties.at("bind_dn_template"));
    }
    else
    {
        throw Exception("Unknown auth type: "+type);
    }
}

}

}
