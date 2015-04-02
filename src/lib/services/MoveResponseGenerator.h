/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _79e7aec7_0b07_42b9_b3a0_5086f1595af9
#define _79e7aec7_0b07_42b9_b3a0_5086f1595af9

#include "ResponseGenerator.h"

namespace dopamine
{

namespace services
{
    
/**
 * @brief Response Generator for C-MOVE services.
 */
class MoveResponseGenerator : public ResponseGenerator
{
public:
    /**
     * Create a default move response generator
     * @param scp: associated C-MOVE SCP
     * @param ouraetitle: Local AE Title
     */
    MoveResponseGenerator(T_ASC_Association * request_association);
    
    /// Destroy the move response generator
    virtual ~MoveResponseGenerator();
    
    /**
     * Callback handler called by the DIMSE_moveProvider callback function
     * @param cancelled: flag indicating whether a C-CANCEL was received (in)
     * @param request: original move request (in)
     * @param requestIdentifiers: original move request identifiers (in)
     * @param responseCount: move response count (in)
     * @param response: final move response (out)
     * @param stDetail: status detail for move response (out)
     * @param responseIdentifiers: move response identifiers (out)
     */
    void process(
        /* in */
        OFBool cancelled, T_DIMSE_C_MoveRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_MoveRSP* response, DcmDataset** stDetail,
        DcmDataset** responseIdentifiers);

    void set_network(T_ASC_Network * network);

protected:

    virtual Uint16 set_query(DcmDataset * dataset);

    /**
     * Process next response
     * @param responseIdentifiers: move response identifiers (out)
     */
    virtual void next(DcmDataset ** responseIdentifiers, DcmDataset ** details);
    
    /**
     * Send a C-Store request with matching Dataset
     * @param sopClassUID: response SOP Class UID
     * @param sopInstanceUID: response SOP Instance UID
     * @param dataset: matching dataset
     * @return EC_Normal if successful, an error code otherwise 
     */
    OFCondition performMoveSubOperation(const char* sopClassUID, 
                                        const char* sopInstanceUID,
                                        DcmDataset* dataset);
                                  
    /**
     * Create an association to send matching data
     * @param request: original move request (in)
     * @return EC_Normal if successful, an error code otherwise 
     */
    OFCondition buildSubAssociation(T_DIMSE_C_MoveRQ* request,
                                    DcmDataset **details);
                            
    /**
     * Add the presentation context
     * @param params: association parameters
     * @return EC_Normal if successful, an error code otherwise 
     */
    OFCondition addAllStoragePresentationContext(T_ASC_Parameters* params);
    
private:
    /// original AE Title
    DIC_AE _origAETitle;
    
    /// Original Message ID
    DIC_US _origMsgID;
    
    /// Association to send C-Store response
    T_ASC_Association * _subAssociation;

    /// Priority of request
    T_DIMSE_Priority _priority;

    T_ASC_Network * _network;

};

} // namespace services
    
} // namespace dopamine

#endif // _79e7aec7_0b07_42b9_b3a0_5086f1595af9
