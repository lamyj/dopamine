/*
 *
 *  Module:  Authenticator
 *
 *  Author:  ICUBE Strasbourg
 *
 *  Purpose: class AuthenticatorCSV
 *
 */
 
#ifndef AUTHENTICATORCSV_H
#define AUTHENTICATORCSV_H

#include <string>
#include <map>

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include <dcmtk/dcmqrdb/dcmqropt.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcmetinf.h>
#include <dcmtk/dcmnet/diutil.h>

#include "AuthenticatorBase.h"

namespace authenticator
{

/** 
 * @brief Authenticator based on a CSV file with two columns (user and 
 * clear-text password).
 */
class AuthenticatorCSV : public AuthenticatorBase
{
public:
    /**
     * Constructor
     * @param ifileName : CSV file path
     */
    AuthenticatorCSV(std::string const & ifileName);
    
    /**
     * Destructor
     */
    virtual ~AuthenticatorCSV();

    /**
     * Operator ()
     * @param identity : User identity
     */
    virtual bool operator()(UserIdentityNegotiationSubItemRQ * identity) const;
    
private:
    std::map<std::string, std::string> _table;
    
};

}

#endif //AUTHENTICATORCSV_H

