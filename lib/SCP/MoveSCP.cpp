/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "MoveResponseGenerator.h"
#include "MoveSCP.h"

namespace research_pacs
{
    
static void moveCallback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_MoveRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_MoveRSP* response, DcmDataset** stDetail,
        DcmDataset** responseIdentifiers)
{
    MoveResponseGenerator* context = reinterpret_cast<MoveResponseGenerator*>(callbackData);
    context->callBackHandler(cancelled, request, requestIdentifiers, 
                             responseCount, response, responseIdentifiers, 
                             stDetail);
}
    
MoveSCP
::MoveSCP(T_ASC_Association * assoc, 
          T_ASC_PresentationContextID presID,
          T_DIMSE_C_MoveRQ * req):
    SCP(assoc, presID), _request(req)
{
    // Nothing to do
}

MoveSCP
::~MoveSCP()
{
    // Nothing to do
}

OFCondition
MoveSCP
::process()
{
    std::cout << "Received Move SCP: MsgID " 
              << this->_request->MessageID << std::endl;

    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(this->_association->params, NULL, aeTitle, NULL);
    
    MoveResponseGenerator context(this, std::string(aeTitle));
    
    return DIMSE_moveProvider(this->_association, this->_presentationID, 
                              this->_request, moveCallback, &context, 
                              DIMSE_BLOCKING, 0);
}
    
} // namespace research_pacs
