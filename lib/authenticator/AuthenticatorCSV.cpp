/*
 *
 *  Module:  Authenticator
 *
 *  Author:  ICUBE Strasbourg
 *
 *  Purpose: class AuthenticatorCSV
 *
 */
 
#include <fstream>

#include <boost/filesystem.hpp>

#include "AuthenticatorCSV.h"
#include "core/ExceptionPACS.h"

namespace authenticator
{
    
/**
 * Constructor
 * Parse a given CSV file and store User/Password as a map
 * @param ifileName : CSV file path
 */
AuthenticatorCSV::AuthenticatorCSV(std::string const & ifileName)
{
    if ( ! boost::filesystem::exists(ifileName.c_str()))
    {
        throw research_pacs::ExceptionPACS("Trying to parse non-existing file.");
    }
    
    // Open file
    std::ifstream stream(ifileName.c_str());
    while(!stream.eof())
    {
        // Store user / password
        std::string user;
        std::string password;
        stream >> user >> password;
        this->_table[user] = password;
    }
}

/**
 * Destructor
 */
AuthenticatorCSV::~AuthenticatorCSV()
{
}

/**
 * Operator ()
 * Search a given User/Password in authorized users map
 * @param identity : User identity
 * @return : true if User exist, false otherwise
 */
bool AuthenticatorCSV::operator ()(UserIdentityNegotiationSubItemRQ & identity) const
{
    bool authorized = false;
    
    // Only available for Identity type: User / Password
    if (identity.getIdentityType() == ASC_USER_IDENTITY_USER_PASSWORD)
    {
        // Get user
        char * user; 
        Uint16 user_length;
        identity.getPrimField(user, user_length);
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
            identity.getSecField(password, password_length);
            
            // password is not NULL-terminated
            std::string passwordstr = std::string(password, password_length);
        
            authorized = it->second == passwordstr;
            
            delete[] password;
        }
        
        delete[] user;
    }
    return authorized;
}

}
