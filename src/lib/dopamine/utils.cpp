/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/utils.h"

#include <string>
#include <odil/AssociationParameters.h>
#include <odil/registry.h>
#include "dopamine/Exception.h"

namespace dopamine
{

std::string
replace(
    std::string const & value, std::string const & old, std::string const & new_)
{
    std::string result(value);
    size_t begin=0;
    while(std::string::npos != (begin=result.find(old, begin)))
    {
        result = result.replace(begin, old.size(), new_);
        if(begin+new_.size()<result.size())
        {
            begin = begin+new_.size();
        }
        else
        {
            begin = std::string::npos;
        }
    }

    return result;
}

std::string get_principal(odil::AssociationParameters const & parameters)
{
    std::string principal;

    auto const identity = parameters.get_user_identity();
    if(identity.type == odil::AssociationParameters::UserIdentity::Type::None)
    {
        principal = "";
    }
    else if(identity.type == odil::AssociationParameters::UserIdentity::Type::Username)
    {
        principal = identity.primary_field;
    }
    else if(identity.type == odil::AssociationParameters::UserIdentity::Type::UsernameAndPassword)
    {
        principal = identity.primary_field;
    }
    else
    {
        throw odil::Exception("Cannot find principal");
    }

    return principal;
}

std::string get_uid_name(std::string const & uid)
{
    auto const it = odil::registry::uids_dictionary.find(uid);
    return (it!=odil::registry::uids_dictionary.end())?it->second.name:uid;
}

} // namespace dopamine
