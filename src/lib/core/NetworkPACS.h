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

class NetworkPACS
{
public:
    static NetworkPACS & get_instance();
    
    static void delete_instance();
    
    virtual ~NetworkPACS();
    
    void run();
    
    T_ASC_Network* get_network() const { return this->_network; };
    
    void force_stop();
    
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
    NetworkPACS();
    
    static NetworkPACS * _instance;
    
    boost::filesystem::path _storage;
    authenticator::AuthenticatorBase * _authenticator;
    
    T_ASC_Network * _network;
    
    bool _forceStop;
    
    int _timeout;
    
    void create_authenticator();

};
    
} // namespace research_pacs

#endif // _95305138_fbe7_4b3a_99d8_9f73013477fd
