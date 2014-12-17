/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _d93218f7_028e_49c7_a2e0_7f3d860331c5
#define _d93218f7_028e_49c7_a2e0_7f3d860331c5

#include "GetSCP.h"
#include "ResponseGenerator.h"

namespace dopamine
{
    
/**
 * @brief Response Generator for C-GET services.
 */
class GetResponseGenerator : public ResponseGenerator
{
public:
    /**
     * Create a default get response generator
     * @param scp: associated C-GET SCP
     * @param ouraetitle: Local AE Title
     */
    GetResponseGenerator(GetSCP * scp, std::string const & ouraetitle);
    
    /// Destroy the get response generator
    virtual ~GetResponseGenerator();
    
    /**
     * Callback handler called by the DIMSE_getProvider callback function
     * @param cancelled: flag indicating whether a C-CANCEL was received (in)
     * @param request: original get request (in)
     * @param requestIdentifiers: original get request identifiers (in)
     * @param responseCount: get response count (in)
     * @param response: final get response (out)
     * @param stDetail: status detail for get response (out)
     * @param responseIdentifiers: get response identifiers (out)
     */
    void callBackHandler(
        /* in */
        OFBool cancelled, T_DIMSE_C_GetRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_GetRSP* response, DcmDataset** stDetail,
        DcmDataset** responseIdentifiers);

protected:
    /**
     * Process next response
     * @param responseIdentifiers: get response identifiers (out)
     */
    virtual void next(DcmDataset ** responseIdentifiers, DcmDataset ** details);
    
    /**
     * Send a C-Store request with matching Dataset
     * @param sopClassUID: response SOP Class UID
     * @param sopInstanceUID: response SOP Instance UID
     * @param dataset: matching dataset
     * @return EC_Normal if successful, an error code otherwise 
     */
    OFCondition performGetSubOperation(const char* sopClassUID, 
                                       const char* sopInstanceUID,
                                       DcmDataset* dataset);

private:
    
};
    
} // namespace dopamine

#endif // _d93218f7_028e_49c7_a2e0_7f3d860331c5
