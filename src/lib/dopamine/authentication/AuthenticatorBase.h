/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _e0761b8c_9889_4e7c_bcf9_8bac78cc353b
#define _e0761b8c_9889_4e7c_bcf9_8bac78cc353b

#include <odil/AssociationParameters.h>

namespace dopamine
{

namespace authentication
{

/// @brief Abstract base class for all authenticators.
class AuthenticatorBase
{
public:
    /// @brief Constructor
    AuthenticatorBase();

    /// @brief Destructor.
    virtual ~AuthenticatorBase() =0;
    
    /// @brief Test authentication based on association parameters.
    virtual bool operator()(
        odil::AssociationParameters const & parameters) const = 0;
};

} // namespace authentication

} // namespace dopamine

#endif // _e0761b8c_9889_4e7c_bcf9_8bac78cc353b
