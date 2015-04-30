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
    RetrieveContext * context =
            reinterpret_cast<RetrieveContext*>(callbackData);

    Uint16 status = STATUS_Pending;

    if (responseCount == 1)
    {
        mongo::BSONObj object = dataset_to_bson(requestIdentifiers);
        if (!object.isValid() || object.isEmpty())
        {
            status = 0xa900;
        }

        // Search into database
        if (status == STATUS_Pending)
        {
            status = context->_generator->set_query(object);
        }

        if (status != STATUS_Pending)
        {
            createStatusDetail(status, DCM_UndefinedTagKey,
                               OFString("An error occured while processing Get operation"),
                               stDetail);
        }
    }

    /* only cancel if we have pending responses */
    if (cancelled && status == STATUS_Pending)
    {
        // Todo: not implemented yet
        context->_generator->cancel();
    }

    /* Process next result */
    if (status == STATUS_Pending)
    {
        mongo::BSONObj object = context->_generator->next();

        if (object.isValid() && object.isEmpty())
        {
            // We're done.
            status = STATUS_Success;
        }
        else
        {
            OFCondition condition =
                    context->_storeprovider->performSubOperation(bson_to_dataset(object),
                                                                 request->Priority);

            if (condition.bad())
            {
                dopamine::loggerError() << "Cannot process sub association: "
                                        << condition.text();
                createStatusDetail(0xc000, DCM_UndefinedTagKey,
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
::GetSCP(T_ASC_Association * assoc, 
         T_ASC_PresentationContextID presID, 
         T_DIMSE_C_GetRQ * req):
    services::SCP(assoc, presID), _request(req) // base class initialisation
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
    dopamine::loggerInfo() << "Received Get SCP: MsgID "
                                << this->_request->MessageID;

    std::string const username =
           get_username(this->_association->params->DULparams.reqUserIdentNeg);
    RetrieveGenerator generator(username);

    StoreSubOperation storeprovider(NULL, this->_association,
                                    this->_request->MessageID);

    RetrieveContext context(&generator, &storeprovider);

    return DIMSE_getProvider(this->_association, this->_presentationID, 
                             this->_request, getCallback, &context, 
                             DIMSE_BLOCKING, 0);
}

} // namespace services

} // namespace dopamine
