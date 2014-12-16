/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include <boost/filesystem.hpp>

#include "AuthenticatorCSV.h"
#include "core/ExceptionPACS.h"

namespace dopamine
{

namespace authenticator
{
    
AuthenticatorCSV
::AuthenticatorCSV(std::string const & ifileName):
    AuthenticatorBase() // base class initialisation
{
    if ( ! boost::filesystem::exists(ifileName.c_str()))
    {
        std::stringstream stream;
        stream << "Trying to parse non-existing file: " << ifileName;
        throw ExceptionPACS(stream.str());
    }
    
    // Open file
    std::ifstream stream(ifileName.c_str());
    while(!stream.eof())
    {
        // Store user / password
        std::string user;
        std::string password;
        stream >> user >> password;
        if (user != "" && password != "")
        {
            if (this->_table.find(user) == this->_table.end())
            {
                this->_table[user] = password;
            }
            //else ignore duplicate key 
        }
        //else ignore empty line
    }
}

AuthenticatorCSV
::~AuthenticatorCSV()
{
}

bool 
AuthenticatorCSV
::operator ()(UserIdentityNegotiationSubItemRQ * identity) const
{
    bool authorized = false;
    
    // Only available if Identity is defined
    if (identity != NULL)
    {
        // Only available for Identity type: User / Password
        if (identity->getIdentityType() == ASC_USER_IDENTITY_USER_PASSWORD)
        {
            // Get user
            char * user; 
            Uint16 user_length;
            identity->getPrimField(user, user_length);
            // user is not NULL-terminated
            std::string userstr = std::string(user, user_length);
            
            // Search in map
            std::map<std::string, std::string>::const_iterator const it = 
                this->_table.find(userstr.c_str());
            // If User exist
            if(it != this->_table.end())
            {
                // Get password
                char * password; Uint16 password_length;
                identity->getSecField(password, password_length);
                
                // password is not NULL-terminated
                std::string passwordstr = std::string(password, password_length);
            
                authorized = it->second == passwordstr;
                
                delete[] password;
            }
            
            delete[] user;
        }
    }
    return authorized;
}

unsigned int 
AuthenticatorCSV
::get_table_count() const
{
    return this->_table.size();
}

} // namespace authenticator

} // namespace dopamine
