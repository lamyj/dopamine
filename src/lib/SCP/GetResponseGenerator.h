/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _d93218f7_028e_49c7_a2e0_7f3d860331c5
#define _d93218f7_028e_49c7_a2e0_7f3d860331c5

#include "GetSCP.h"
#include "ResponseGenerator.h"

namespace research_pacs
{
    
class GetResponseGenerator : public ResponseGenerator
{
public:
    GetResponseGenerator(GetSCP * scp, std::string const & ouraetitle);
    
    virtual ~GetResponseGenerator();
    
    void callBackHandler(
        /* in */
        OFBool cancelled, T_DIMSE_C_GetRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_GetRSP* response, DcmDataset** stDetail,
        DcmDataset** responseIdentifiers);

protected:
    virtual void next(DcmDataset ** responseIdentifiers);
    
    OFCondition performGetSubOperation(const char* sopClassUID, 
                                       const char* sopInstanceUID,
                                       DcmDataset* dataset);

private:
    
};
    
} // namespace research_pacs

#endif // _d93218f7_028e_49c7_a2e0_7f3d860331c5
