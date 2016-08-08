/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _933cb005_91e0_4ba0_a8e6_3f4fb0612d19
#define _933cb005_91e0_4ba0_a8e6_3f4fb0612d19

#include <string>

#include <odil/AssociationParameters.h>

#include "dopamine/authentication/AuthenticatorBase.h"

namespace dopamine
{

namespace authentication
{

/**
 * @brief Authenticator based on an LDAP directory.
 *
 * This authenticator tries to bind to the specified LDAP uri with a bind DN
 * derived from the user name and the password from the association.
 *
 * The bind DN template can contain "%user", which will be replaced with the
 * user name.
 */
class AuthenticatorLDAP: public AuthenticatorBase
{
public:
    /// @brief Constructor.
    AuthenticatorLDAP(
        std::string const & uri, std::string const & bind_dn_template);

    /// @brief Destructor.
    virtual ~AuthenticatorLDAP();

    /// @brief Try to bind with the user name and password to an LDAP directory.
    virtual bool operator()(
        odil::AssociationParameters const & parameters) const;

private:
    std::string _uri;
    std::string _bind_dn_template;
};

} // namespace authentication

} // namespace dopamine

#endif // _933cb005_91e0_4ba0_a8e6_3f4fb0612d19
