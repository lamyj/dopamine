/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _e0761b8c_9889_4e7c_bcf9_8bac78cc353b
#define _e0761b8c_9889_4e7c_bcf9_8bac78cc353b

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include <dcmtk/dcmqrdb/dcmqropt.h>

namespace dopamine
{

namespace authenticator
{

/// @brief Abstract base class for all authenticators.
class AuthenticatorBase
{
public:
    /**
     * Create a default Authenticator
     */
    AuthenticatorBase() {}

    /**
     * Destroy the authenticator
     */
    virtual ~AuthenticatorBase() {}
    
    /**
     * Operator ()
     * @param identity: requested authentication
     * @return true if authentication success, false otherwise
     */
    virtual bool operator()(UserIdentityNegotiationSubItemRQ * identity) const =0;
};

} // namespace authenticator

} // namespace dopamine

#endif // _e0761b8c_9889_4e7c_bcf9_8bac78cc353b
