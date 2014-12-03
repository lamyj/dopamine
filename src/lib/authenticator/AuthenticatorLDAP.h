/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _933cb005_91e0_4ba0_a8e6_3f4fb0612d19
#define _933cb005_91e0_4ba0_a8e6_3f4fb0612d19

#include "AuthenticatorBase.h"

namespace authenticator
{

/**
 * @brief The AuthenticatorLDAP class
 */
class AuthenticatorLDAP : public AuthenticatorBase
{
public:
    /// Create an instance of AuthenticatorLDAP
    AuthenticatorLDAP();

    /// Destroy the instance of AuthenticatorLDAP
    virtual ~AuthenticatorLDAP();

    /**
     * Operator ()
     * @param identity: requested authentication
     * @return true if authentication success, false otherwise
     */
    virtual bool operator()(UserIdentityNegotiationSubItemRQ * identity) const;

private:
    std::string _ldap_server;

    std::string _ldap_bind_user;

    std::string _ldap_base;

    std::string _ldap_filter;

};

} // namespace authenticator

#endif // _933cb005_91e0_4ba0_a8e6_3f4fb0612d19
