/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/authentication/AuthenticatorCSV.h"

#include <fstream>
#include <map>
#include <string>

#include <boost/filesystem.hpp>
#include <odil/AssociationParameters.h>

#include "dopamine/authentication/AuthenticatorBase.h"
#include "dopamine/Exception.h"

namespace dopamine
{

namespace authentication
{
    
AuthenticatorCSV
::AuthenticatorCSV(std::string const & path):
    AuthenticatorBase() // base class initialisation
{
    if(!boost::filesystem::exists(path.c_str()))
    {
        throw Exception("Trying to parse non-existing file: " + path);
    }
    
    // Open file
    std::ifstream stream(path);
    while(!stream.eof())
    {
        // Store user / password
        std::string user;
        std::string password;
        stream >> user >> password;
        if(user != "" && password != "")
        {
            this->_table[user] = password;
        }
    }
    stream.close();
}

AuthenticatorCSV
::~AuthenticatorCSV()
{
    // Nothing to do
}

bool
AuthenticatorCSV
::operator()(odil::AssociationParameters const & parameters) const
{
    bool authenticated = false;

    // Only available for Identity type: User / Password
    if(parameters.get_user_identity().type ==
        odil::AssociationParameters::UserIdentity::Type::UsernameAndPassword)
    {
        auto const & username =
            parameters.get_user_identity().primary_field;
        auto const & password =
            parameters.get_user_identity().secondary_field;

        auto const it = this->_table.find(username);
        if(it != this->_table.end())
        {
            authenticated = (it->second == password);
        }
    }

    return authenticated;
}

} // namespace authentication

} // namespace dopamine
