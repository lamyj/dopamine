/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _cd33d64f_50b1_40a7_9288_e90b85b9a576
#define _cd33d64f_50b1_40a7_9288_e90b85b9a576

#include <map>
#include <string>

#include <odil/AssociationParameters.h>

#include "dopamine/authentication/AuthenticatorBase.h"

namespace dopamine
{

namespace authentication
{

/**
 * @brief Authenticator based on a CSV file with two columns
 * (user name and clear-text password).
 */
class AuthenticatorCSV: public AuthenticatorBase
{
public:
    /// @brief Constructor from a CSV file.
    AuthenticatorCSV(std::string const & path);
    
    /// @brief Destructor.
    virtual ~AuthenticatorCSV();

    /// @brief Look up the user name and password in the CSV file
    virtual bool operator()(
        odil::AssociationParameters const & parameters) const;
private:
    /// User - Password dictionary
    std::map<std::string, std::string> _table;
    
};

} // namespace authentication

} // namespace dopamine

#endif // _cd33d64f_50b1_40a7_9288_e90b85b9a576
