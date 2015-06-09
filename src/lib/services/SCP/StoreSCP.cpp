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
static void store_callback(
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
            create_status_detail(STATUS_STORE_Refused_SOPClassNotSupported,
                                 DCM_UndefinedTagKey,
                                 OFString("An error occured while processing Storage"),
                                 stDetail);
        }
        else
        {
            StoreGenerator* context =
                    reinterpret_cast<StoreGenerator*>(callbackData);
            Uint16 status = context->process_dataset(*imageDataSet, true);

            if (status != STATUS_Pending)
            {
                rsp->DimseStatus = status;
                create_status_detail(status, DCM_UndefinedTagKey,
                                     OFString("An error occured while processing Storage"),
                                     stDetail);
            }
            else
            {
                status = STATUS_Success;
            }
        }

    }
}

StoreSCP
::StoreSCP(T_ASC_Association * association,
           T_ASC_PresentationContextID presentation_context_id,
           T_DIMSE_C_StoreRQ * request):
    SCP(association, presentation_context_id), // base class initialisation
    _request(request)
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
    dopamine::logger_info() << "Received Store SCP: MsgID "
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
    context.set_calling_aptitle(callingaptitle);

    /* we must still retrieve the data set even if some error has occured */
    DcmDataset dset;
    DcmDataset * dset_ptr = &dset;
    return DIMSE_storeProvider(this->_association, this->_presentation_context_id,
                               this->_request, (char *)NULL, 1,
                               &dset_ptr, store_callback, (void*)&context,
                               DIMSE_BLOCKING, 0);
}

} // namespace services

} // namespace dopamine
