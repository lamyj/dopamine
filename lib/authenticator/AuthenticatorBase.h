/*
 *
 *  Module:  Authenticator
 *
 *  Author:  ICUBE Strasbourg
 *
 *  Purpose: class AuthenticatorBase
 *
 */
 
#ifndef AUTHENTICATORBASE_H
#define AUTHENTICATORBASE_H

namespace authenticator
{

/// @brief Abstract base class for all authenticators.
class AuthenticatorBase
{
public:
    /**
     * Destructor
     */
    virtual ~AuthenticatorBase() {}
    
    /**
     * Operator ()
     */
    virtual bool operator()(UserIdentityNegotiationSubItemRQ & identity) const =0;
};

}

#endif //AUTHENTICATORBASE_H
