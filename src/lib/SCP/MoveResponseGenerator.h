/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _79e7aec7_0b07_42b9_b3a0_5086f1595af9
#define _79e7aec7_0b07_42b9_b3a0_5086f1595af9

#include "MoveSCP.h"
#include "ResponseGenerator.h"

namespace research_pacs
{
    
class MoveResponseGenerator : public ResponseGenerator
{
public:
    MoveResponseGenerator(MoveSCP * scp, std::string const & ouraetitle);
    
    virtual ~MoveResponseGenerator();
    
    void callBackHandler(
        /* in */
        OFBool cancelled, T_DIMSE_C_MoveRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_MoveRSP* response, DcmDataset** stDetail,
        DcmDataset** responseIdentifiers);

protected:
    virtual void next(DcmDataset ** responseIdentifiers);
    
    OFCondition performMoveSubOperation(const char* sopClassUID, 
                                        const char* sopInstanceUID,
                                        DcmDataset* dataset);
                                        
    OFCondition buildSubAssociation(T_DIMSE_C_MoveRQ* request);
                            
    OFCondition addAllStoragePresentationContext(T_ASC_Parameters* params);
    
private:
    DIC_AE _origAETitle;
    DIC_US _origMsgID;
    
    T_ASC_Association * _subAssociation;

};
    
} // namespace research_pacs

#endif // _79e7aec7_0b07_42b9_b3a0_5086f1595af9
