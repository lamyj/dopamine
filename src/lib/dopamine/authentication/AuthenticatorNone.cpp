/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "AuthenticatorNone.h"

namespace dopamine
{

namespace authenticator
{

AuthenticatorNone
::AuthenticatorNone():
    AuthenticatorBase() // base class initialisation
{
    // Nothing to do
}

AuthenticatorNone
::~AuthenticatorNone()
{
    // Nothing to do
}

bool
AuthenticatorNone
::operator()(dcmtkpp::DcmtkAssociation const & association) const
{
    // Only available for Identity type: None
    return association.get_user_identity_type() ==
           dcmtkpp::UserIdentityType::None;
}

} // namespace authenticator

} // namespace dopamine
