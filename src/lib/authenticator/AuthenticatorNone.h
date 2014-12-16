/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _78b49719_8a47_493e_8e12_92848f9bccbe
#define _78b49719_8a47_493e_8e12_92848f9bccbe

#include "AuthenticatorBase.h"

namespace dopamine
{

namespace authenticator
{

/**
 * @brief The AuthenticatorNone class
 */
class AuthenticatorNone : public AuthenticatorBase
{
public:
    /// Create an instance of AuthenticatorNone
    AuthenticatorNone();

    /// Destroy the instance of AuthenticatorNone
    virtual ~AuthenticatorNone();

    /**
     * @brief operator ()
     * @param identity: requested authentication
     * @return true if authentication success, false otherwise
     */
    virtual bool operator()(UserIdentityNegotiationSubItemRQ * identity) const;

};

} // namespace authenticator

} // namespace dopamine

#endif // _78b49719_8a47_493e_8e12_92848f9bccbe
