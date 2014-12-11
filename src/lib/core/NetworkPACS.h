/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _95305138_fbe7_4b3a_99d8_9f73013477fd
#define _95305138_fbe7_4b3a_99d8_9f73013477fd

#include <boost/filesystem.hpp>

#include "authenticator/AuthenticatorCSV.h"
#include "authenticator/AuthenticatorLDAP.h"
#include "authenticator/AuthenticatorNone.h"


namespace research_pacs
{
    
/// enumeration describing reasons for refusing an association request
enum CTN_RefuseReason
{
    /// too many concurrent associations
    CTN_TooManyAssociations,
    /// fork system function failed
    CTN_CannotFork,
    /// bad application context (not DICOM)
    CTN_BadAppContext,
    /// unknown peer application entity title (access not authorised)
    CTN_BadAEPeer,
    /// unknown peer application entity title (access not authorised)
    CTN_BadAEService,
    /// other non-specific reason
    CTN_NoReason
};

/**
 * @brief Class to create and manage a Network
 */
class NetworkPACS
{
public:
    /**
     * Create (if not exist) and return an unique instance of NetworkPACS
     * @return unique instance of NetworkPACS
     */
    static NetworkPACS & get_instance();
    
    /**
     * Remove the unique instance of NetworkPACS
     */
    static void delete_instance();
    
    /// Destroy the Network
    virtual ~NetworkPACS();
    
    /// While loop to listen the network
    void run();
    
    /**
     * Get the created network
     * @return current network
     */
    T_ASC_Network* get_network() const { return this->_network; }
    
    /**
     * Stop running after the next received association or time out
     */
    void force_stop();
    
    /**
     * Set the waiting time out
     * @param new time-out value (in second)
     */
    void set_timeout(int const & timeout) { this->_timeout = timeout; }

protected:
    /** perform association negotiation for an incoming A-ASSOCIATE request based
     *  on the SCP configuration and option flags. No A-ASSOCIATE response is generated,
     *  this is left to the caller.
     *  @param assoc incoming association
     *  @return EC_Normal if successful, an error code otherwise
     */
    OFCondition negotiateAssociation(T_ASC_Association * assoc);
  
    void refuseAssociation(T_ASC_Association ** assoc, CTN_RefuseReason reason);
    
    void handleAssociation(T_ASC_Association * assoc);

private:
    /// Create an instance of NetworkPACS and initialize the network
    NetworkPACS();
    
    /// Unique Instance
    static NetworkPACS * _instance;
    
    /// Authenticator manager
    authenticator::AuthenticatorBase * _authenticator;
    
    /// Network for listening/sending Requests and Responses
    T_ASC_Network * _network;
    
    /// flag indicating if while loop should be Stop
    bool _forceStop;
    
    /// Waiting time-out
    int _timeout;
    
    /// Initialize the authenticator manager
    void create_authenticator();

};
    
} // namespace research_pacs

#endif // _95305138_fbe7_4b3a_99d8_9f73013477fd
