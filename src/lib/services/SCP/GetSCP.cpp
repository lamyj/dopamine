/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "core/LoggerPACS.h"
#include "GetSCP.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace services
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
static void get_callback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_GetRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_GetRSP *response, 
        DcmDataset **stDetail,
        DcmDataset **responseIdentifiers)
{
    RetrieveContext * context =
            reinterpret_cast<RetrieveContext*>(callbackData);

    Uint16 status = STATUS_Pending;

    if (responseCount == 1)
    {
        status = context->get_generator()->process_dataset(requestIdentifiers, false);

        if (status != STATUS_Pending)
        {
            create_status_detail(status, DCM_UndefinedTagKey,
                                 OFString("An error occured while processing Get operation"),
                                 stDetail);
        }
    }

    /* only cancel if we have pending responses */
    if (cancelled && status == STATUS_Pending)
    {
        // Todo: not implemented yet
        context->get_generator()->cancel();
    }

    /* Process next result */
    if (status == STATUS_Pending)
    {
        mongo::BSONObj const object = context->get_generator()->next();

        if (object.isValid() && object.isEmpty())
        {
            // We're done.
            status = STATUS_Success;
        }
        else
        {
            OFCondition condition =
                context->get_storeprovider()->perform_sub_operation(context->get_generator()->retrieve_dataset(object),
                                                                    request->Priority);

            if (condition.bad())
            {
                dopamine::logger_error() << "Cannot process sub association: "
                                         << condition.text();
                create_status_detail(0xc000, DCM_UndefinedTagKey,
                                     OFString(condition.text()), stDetail);

                status = 0xc000; // Unable to process
            }
            // else status = STATUS_Pending
        }
    }

    /* set response status */
    response->DimseStatus = status;
    if (status == STATUS_Pending || status == STATUS_Success)
    {
        (*stDetail) = NULL;
    }
}
    
GetSCP
::GetSCP(T_ASC_Association * association,
         T_ASC_PresentationContextID presentation_context_id,
         T_DIMSE_C_GetRQ * request):
    services::SCP(association, presentation_context_id), // base class initialisation
    _request(request)
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
    dopamine::logger_info() << "Received Get SCP: MsgID "
                                << this->_request->MessageID;

    std::string const username =
           get_username(this->_association->params->DULparams.reqUserIdentNeg);
    RetrieveGenerator generator(username);

    StoreSubOperation storeprovider(NULL, this->_association,
                                    this->_request->MessageID);

    RetrieveContext context(&generator, &storeprovider);

    return DIMSE_getProvider(this->_association, this->_presentation_context_id,
                             this->_request, get_callback, &context,
                             DIMSE_BLOCKING, 0);
}

} // namespace services

} // namespace dopamine
