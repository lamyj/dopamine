/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/authentication/AuthenticatorLDAP.h"

#include <string>

#include <ldap.h>
#include <odil/AssociationParameters.h>

#include "dopamine/authentication/AuthenticatorBase.h"
#include "dopamine/Exception.h"
#include "dopamine/utils.h"

namespace dopamine
{

namespace authentication
{

AuthenticatorLDAP
::AuthenticatorLDAP(
    std::string const & uri, std::string const & bind_dn_template)
: _uri(uri), _bind_dn_template(bind_dn_template)
{
    // Nothing to do.
}

AuthenticatorLDAP
::~AuthenticatorLDAP()
{
    // Nothing to do.
}

bool
AuthenticatorLDAP
::operator()(odil::AssociationParameters const & parameters) const
{
    bool authenticated = false;

    // Only available for Identity type: User / Password
    if(parameters.get_user_identity().type ==
        odil::AssociationParameters::UserIdentity::Type::UsernameAndPassword)
    {
        LDAP * session;

        auto const initialize_ok = ldap_initialize(&session, this->_uri.c_str());
        if(initialize_ok != LDAP_SUCCESS)
        {
            throw Exception(
                std::string("ldap_initialize error: ")
                + ldap_err2string(initialize_ok));
        }

        auto const & username = parameters.get_user_identity().primary_field;
        auto const bind_dn = replace(
            this->_bind_dn_template, "%user", username);

        auto const & password = parameters.get_user_identity().secondary_field;
        berval credentials;
        credentials.bv_val = const_cast<char*>(&password[0]);
        credentials.bv_len = password.size();

        /* User authentication (bind) */
        auto const bind_ok = ldap_sasl_bind_s(
            session, bind_dn.c_str(), NULL, &credentials, NULL, NULL,NULL);
        if(bind_ok == LDAP_INVALID_CREDENTIALS)
        {
            authenticated = false;
            ldap_destroy(session);
        }
        else if(bind_ok != LDAP_SUCCESS)
        {
            ldap_destroy(session);
            throw Exception(
                std::string("ldap_sasl_bind_s error: ")
                + ldap_err2string(bind_ok));
        }
        else
        {
            authenticated = true;
            ldap_unbind_ext_s(session, NULL, NULL);
            // No need to call ldap_destroy, unbinding frees the session
        }
    }

    return authenticated;
}

} // namespace authentication

} // namespace dopamine
