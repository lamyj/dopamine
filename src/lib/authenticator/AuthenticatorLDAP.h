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

#include "AuthenticatorBase.h"

namespace dopamine
{

namespace authenticator
{

/**
 * @brief \class The AuthenticatorLDAP class
 */
class AuthenticatorLDAP : public AuthenticatorBase
{
public:
    /**
     * @brief Create an instance of AuthenticatorLDAP
     * @param ldap_server
     * @param ldap_bind_user
     * @param ldap_base
     * @param ldap_filter
     */
    AuthenticatorLDAP(std::string const & ldap_server,
                      std::string const & ldap_bind_user,
                      std::string const & ldap_base,
                      std::string const & ldap_filter);

    /// Destroy the instance of AuthenticatorLDAP
    virtual ~AuthenticatorLDAP();

    /**
     * Operator ()
     * @param identity: requested authentication
     * @return true if authentication success, false otherwise
     */
    virtual bool operator()(dcmtkpp::DcmtkAssociation const & association) const;

private:
    ///
    std::string _ldap_server;

    ///
    std::string _ldap_bind_user;

    ///
    std::string _ldap_base;

    ///
    std::string _ldap_filter;

};

} // namespace authenticator

} // namespace dopamine

#endif // _933cb005_91e0_4ba0_a8e6_3f4fb0612d19
