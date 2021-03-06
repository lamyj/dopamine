/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _78b49719_8a47_493e_8e12_92848f9bccbe
#define _78b49719_8a47_493e_8e12_92848f9bccbe

#include <odil/AssociationParameters.h>

#include "dopamine/authentication/AuthenticatorBase.h"

namespace dopamine
{

namespace authentication
{

/// @brief Authenticator succeeding for all associations
class AuthenticatorNone: public AuthenticatorBase
{
public:
    /// @brief Constructor.
    AuthenticatorNone();

    /// @brief Destructor.
    virtual ~AuthenticatorNone();

    /// @brief Always succeed.
    virtual bool operator()(
        odil::AssociationParameters const & parameters) const;

};

} // namespace authentication

} // namespace dopamine

#endif // _78b49719_8a47_493e_8e12_92848f9bccbe
