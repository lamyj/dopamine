/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/conversion.h>
#include <dcmtkpp/DataSet.h>
#include <dcmtkpp/message/Response.h>
#include <dcmtkpp/Tag.h>

#include "core/LoggerPACS.h"
#include "MoveSCP.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace services
{

/**
 * Callback handler called by the DIMSE_moveProvider callback function
 * @param callbackdata: Callback context (in)
 * @param cancelled: flag indicating wheter a C-CANCEL was received (in)
 * @param request: original move request (in)
 * @param requestIdentifiers: original move request identifiers (in)
 * @param responseCount: move response count (in)
 * @param response: final move response (out)
 * @param stDetail: status detail for move response (out)
 * @param responseIdentifiers: move response identifiers (out)
 */
static void move_callback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_MoveRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_MoveRSP* response,
        DcmDataset** stDetail, DcmDataset** responseIdentifiers)
{
    RetrieveContext * context =
            reinterpret_cast<RetrieveContext*>(callbackData);

    Uint16 status = dcmtkpp::message::Response::Pending;
    dcmtkpp::DataSet details;

    if (responseCount == 1)
    {
        status = context->get_generator()->process_dataset(dcmtkpp::convert(requestIdentifiers),
                                                           false);

        if (status != dcmtkpp::message::Response::Pending)
        {
            details = create_status_detail(
                    status, dcmtkpp::Tag(0xffff, 0xffff),
                    "An error occured while processing Move operation");
        }

        // Create Move SubAssociation
        OFCondition condition =
                context->get_storeprovider()->build_sub_association(
                                request->MoveDestination);
        if (condition.bad())
        {
            dopamine::logger_error() << "Cannot create sub association: "
                                    << condition.text();
            details = create_status_detail(
                    0xc000, dcmtkpp::Tag(0xffff, 0xffff),
                    condition.text());

            status = 0xc000; // Unable to process
        }
    }

    /* only cancel if we have pending responses */
    if (cancelled && status == dcmtkpp::message::Response::Pending)
    {
        // Todo: not implemented yet
        context->get_generator()->cancel();
    }

    /* Process next result */
    if (status == dcmtkpp::message::Response::Pending)
    {
        mongo::BSONObj const object = context->get_generator()->next();

        if (object.isValid() && object.isEmpty())
        {
            // We're done.
            status = dcmtkpp::message::Response::Success;
        }
        else
        {
            OFCondition condition =
                context->get_storeprovider()->perform_sub_operation(
                        context->get_generator()->retrieve_dataset(object),
                        request->Priority);

            if (condition.bad())
            {
                dopamine::logger_error() << "Cannot process sub association: "
                                         << condition.text();
                details = create_status_detail(
                        0xc000, dcmtkpp::Tag(0xffff, 0xffff),
                        condition.text());

                status = 0xc000; // Unable to process
            }
            // else status = dcmtkpp::message::Response::Pending
        }
    }

    /* set response status */
    response->DimseStatus = status;
    if (status == dcmtkpp::message::Response::Pending || status == dcmtkpp::message::Response::Success)
    {
        (*stDetail) = NULL;
    }
    else
    {
        (*stDetail) = dynamic_cast<DcmDataset*>(dcmtkpp::convert(details, true));
    }
}
    
MoveSCP
::MoveSCP(T_ASC_Association * association,
          T_ASC_PresentationContextID presentation_context_id,
          T_DIMSE_C_MoveRQ * request):
    services::SCP(association, presentation_context_id),
    _request(request), _network(NULL)
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
   dopamine::logger_info() << "Received Move SCP: MsgID "
                               << this->_request->MessageID;

    std::string const username =
           get_username(this->_association->params->DULparams.reqUserIdentNeg);
    RetrieveGenerator generator(username);

    StoreSubOperation storeprovider(this->_network, this->_association,
                                    this->_request->MessageID);

    RetrieveContext context(&generator, &storeprovider);
    
    return DIMSE_moveProvider(this->_association, this->_presentation_context_id,
                              this->_request, move_callback, &context,
                              DIMSE_BLOCKING, 0);
}

void
MoveSCP
::set_network(T_ASC_Network *network)
{
    this->_network = network;
}

} // namespace services
    
} // namespace dopamine
