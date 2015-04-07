/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "core/LoggerPACS.h"
#include "services/ServicesTools.h"
#include "services/StoreGenerator.h"
#include "StoreSCP.h"

namespace dopamine
{

namespace services
{

/**
 * Callback handler called by the DIMSE_storeProvider callback function
 * @param callbackdata: Callback context (in)
 * @param progress: progress state (in)
 * @param req: original store request (in)
 * @param imageFileName: being received into (in)
 * @param imageDataSet: being received into (in)
 * @param rsp: final store response (out)
 * @param stDetail: status detail for find response (out)
 */
static void storeCallback(
    /* in */
    void *callbackData,
    T_DIMSE_StoreProgress *progress,    /* progress state */
    T_DIMSE_C_StoreRQ *req,             /* original store request */
    char *imageFileName,                /* being received into */
    DcmDataset **imageDataSet,          /* being received into */
    /* out */
    T_DIMSE_C_StoreRSP *rsp,            /* final store response */
    DcmDataset **stDetail)
{
    if(progress->state == DIMSE_StoreEnd)
    {
        if (!dcmIsaStorageSOPClassUID(req->AffectedSOPClassUID))
        {
            /* callback will send back sop class not supported status */
            rsp->DimseStatus = STATUS_STORE_Refused_SOPClassNotSupported;
            createStatusDetail(STATUS_STORE_Refused_SOPClassNotSupported,
                               DCM_UndefinedTagKey,
                               OFString("An error occured while processing Storage"),
                               stDetail);
        }
        else
        {
            StoreGenerator* context =
                    reinterpret_cast<StoreGenerator*>(callbackData);

            Uint16 result = context->set_query(*imageDataSet);

            if (result != STATUS_Success)
            {
                rsp->DimseStatus = result;
                createStatusDetail(result, DCM_UndefinedTagKey,
                                   OFString("An error occured while processing Storage"),
                                   stDetail);
            }
        }

    }
}

StoreSCP
::StoreSCP(T_ASC_Association * assoc, 
           T_ASC_PresentationContextID presID, 
           T_DIMSE_C_StoreRQ * req):
    SCP(assoc, presID), _request(req) // base class initialisation
{
    // nothing to do
}

StoreSCP
::~StoreSCP()
{
    // nothing to do
}

OFCondition 
StoreSCP
::process()
{
    dopamine::loggerInfo() << "Received Store SCP: MsgID "
                                << this->_request->MessageID;

    std::string const username =
           get_username(this->_association->params->DULparams.reqUserIdentNeg);
    StoreGenerator context(username);

    std::string callingaptitle = "";
    char const * aet = this->_association->params->DULparams.callingAPTitle;
    if(aet != NULL)
    {
        callingaptitle = this->_association->params->DULparams.callingAPTitle;
    }
    context.set_callingaptitle(callingaptitle);

    /* we must still retrieve the data set even if some error has occured */
    DcmDataset dset;
    DcmDataset * dset_ptr = &dset;
    return DIMSE_storeProvider(this->_association, this->_presentationID, 
                               this->_request, (char *)NULL, 1,
                               &dset_ptr, storeCallback, (void*)&context,
                               DIMSE_BLOCKING, 0);
}

} // namespace services

} // namespace dopamine
