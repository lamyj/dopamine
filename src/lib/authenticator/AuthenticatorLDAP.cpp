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
::operator()(odil::AssociationParameters const & parameters) const
{
    bool return_ = false;

    // Only available for Identity type: User / Password
    if (parameters.get_user_identity().type ==
            odil::AssociationParameters::UserIdentity::Type::UsernameAndPassword)
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

        auto const username = parameters.get_user_identity().primary_field;
        auto const pwd = parameters.get_user_identity().secondary_field;

        std::string bind_dn = this->_ldap_bind_user;
        boost::replace_all(bind_dn, "%user", username.c_str());

        char* password = new char[pwd.size()];
        strncpy(password, &pwd[0], pwd.size());

        // Password
        berval credential;
        credential.bv_val = password;
        credential.bv_len = pwd.size();

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
        boost::replace_all(filter, "%user", username.c_str());

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
