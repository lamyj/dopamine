/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h>    /* make sure OS specific configuration is included first */
#include <dcmtk/dcmqrdb/dcmqropt.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmnet/diutil.h>

#include "GetResponseGenerator.h"
#include "GetSCP.h"

namespace research_pacs
{

/**
 * Callback handler called by the DIMSE_getProvider callback function
 * @param callbackdata: Callback context (in)
 * @param cancelled: flag indicating wheter a C-CANCEL was received (in)
 * @param request: original get request (in)
 * @param requestIdentifiers: original get request identifiers (in)
 * @param responseCount: get response count (in)
 * @param response: final get response (out)
 * @param stDetail: status detail for get response (out)
 * @param responseIdentifiers: get response identifiers (out)
 */
static void getCallback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_GetRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_GetRSP *response, 
        DcmDataset **stDetail,
        DcmDataset **responseIdentifiers)
{
    GetResponseGenerator* context = reinterpret_cast<GetResponseGenerator*>(callbackData);
    context->callBackHandler(cancelled, request, requestIdentifiers, 
                             responseCount, response, responseIdentifiers, 
                             stDetail);
}
    
GetSCP
::GetSCP(T_ASC_Association * assoc, 
         T_ASC_PresentationContextID presID, 
         T_DIMSE_C_GetRQ * req):
    SCP(assoc, presID), _request(req) // base class initialisation
{
    // nothing to do
}

GetSCP
::~GetSCP()
{
    // nothing to do
}

OFCondition 
GetSCP
::process()
{
    std::cout << "Received Get SCP: MsgID " 
              << this->_request->MessageID << std::endl;

    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(this->_association->params, NULL, aeTitle, NULL);
    
    GetResponseGenerator context(this, std::string(aeTitle));
    
    return DIMSE_getProvider(this->_association, this->_presentationID, 
                             this->_request, getCallback, &context, 
                             DIMSE_BLOCKING, 0);
}

} // namespace research_pacs
