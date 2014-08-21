/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _ee9915a2_a504_4b21_8d43_7938c66c526e
#define _ee9915a2_a504_4b21_8d43_7938c66c526e

#include "FindSCP.h"
#include "ResponseGenerator.h"

namespace research_pacs
{

/**
 * @brief Response Generator for C-FIND services.
 */
class FindResponseGenerator : public ResponseGenerator
{
public :
    typedef FindResponseGenerator Self;

    /**
     * Create a default find response generator
     * @param scp: associated C-FIND SCP
     * @param ouraetitle: Local AE Title
     */
    FindResponseGenerator(FindSCP* scp, std::string const & ouraetitle);
    
    /// Destroy the find response generator
    virtual ~FindResponseGenerator();
    
    /**
     * Callback handler called by the DIMSE_findProvider callback function
     * @param cancelled: flag indicating whether a C-CANCEL was received (in)
     * @param request: original find request (in)
     * @param requestIdentifiers: original find request identifiers (in)
     * @param responseCount: find response count (in)
     * @param response: final find response (out)
     * @param stDetail: status detail for find response (out)
     * @param responseIdentifiers: find response identifiers (out)
     */
    void callBackHandler(
        /* in */
        OFBool cancelled, T_DIMSE_C_FindRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_FindRSP* response, DcmDataset** stDetail,
        DcmDataset** responseIdentifiers);
    
protected:
    /**
     * Process next response
     * @param responseIdentifiers: find response identifiers (out)
     */
    virtual void next(DcmDataset ** responseIdentifiers);

private :
    /// flag indicating if modalities should be convert
    bool _convert_modalities_in_study;
    
};

} // namespace research_pacs

#endif // _ee9915a2_a504_4b21_8d43_7938c66c526e
