/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include <boost/algorithm/string/replace.hpp>

#include <ldap.h>

#include "AuthenticatorLDAP.h"
#include "core/ConfigurationPACS.h"
#include "core/ExceptionPACS.h"

namespace dopamine
{

namespace authenticator
{

AuthenticatorLDAP
::AuthenticatorLDAP(std::string const & ldap_server,
                    std::string const & ldap_bind_user,
                    std::string const & ldap_base,
                    std::string const & ldap_filter):
    AuthenticatorBase(), // base class initialisation
    _ldap_server(ldap_server), _ldap_bind_user(ldap_bind_user),
    _ldap_base(ldap_base), _ldap_filter(ldap_filter)
{
    // Nothing to do
}

AuthenticatorLDAP
::~AuthenticatorLDAP()
{
    // Nothing to do
}

bool
AuthenticatorLDAP
::operator()(UserIdentityNegotiationSubItemRQ *identity) const
{
    // Not available if Identity is not defined
    if (identity == NULL)
    {
        return false;
    }

    bool return_ = false;

    // Only available for Identity type: User / Password
    if (identity->getIdentityType() == ASC_USER_IDENTITY_USER_PASSWORD)
    {
        LDAP *ld;

        /* Open LDAP Connection */
        int rc = ldap_initialize( &ld, this->_ldap_server.c_str() );
        if( rc != LDAP_SUCCESS )
        {
            std::stringstream stream;
            stream << "ldap_initialize error: " << ldap_err2string(rc);
            throw ExceptionPACS(stream.str());
        }

        // Get user
        char * user;
        Uint16 user_length;
        identity->getPrimField(user, user_length);
        // user is not NULL-terminated
        std::string const userstr = std::string(user, user_length);

        std::string bind_dn = this->_ldap_bind_user;
        boost::replace_all(bind_dn, "%user", userstr.c_str());

        // Get password
        char * password;
        Uint16 password_length;
        identity->getSecField(password, password_length);

        // Password
        berval credential;
        credential.bv_val = password;
        credential.bv_len = password_length;

        /* User authentication (bind) */
        rc = ldap_sasl_bind_s( ld, bind_dn.c_str(), NULL, &credential,
                               NULL, NULL,NULL);
        if( rc != LDAP_SUCCESS )
        {
            // Remove connection
            ldap_destroy(ld);

            std::stringstream stream;
            stream << "ldap_sasl_bind_s error: " << ldap_err2string(rc);
            throw ExceptionPACS(stream.str());
        }

        std::string filter = this->_ldap_filter;
        boost::replace_all(filter, "%user", userstr.c_str());

        LDAPMessage * msg;
        // Request
        rc = ldap_search_ext_s(ld, this->_ldap_base.c_str(), LDAP_SCOPE_SUBTREE,
                               filter.c_str(), NULL, 0,
                               NULL, NULL, NULL, 1024, &msg);
        if (rc != LDAP_SUCCESS)
        {
            std::stringstream stream;
            stream << "ldap_search_ext_s error: " << ldap_err2string(rc);
            throw ExceptionPACS(stream.str());
        }
        else
        {
            return_ = (ldap_count_entries(ld, msg) == 1);
        }

        // Unbind and remove connection
        rc = ldap_unbind_ext_s( ld, NULL, NULL);
        if (rc != LDAP_SUCCESS)
        {
            std::stringstream stream;
            stream << "ldap_unbind_ext_s error: " << ldap_err2string(rc);
            throw ExceptionPACS(stream.str());
        }
    }
    return return_;
}

} // namespace authenticator

} // namespace dopamine
