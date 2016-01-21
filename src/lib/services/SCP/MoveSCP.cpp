/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <set>
#include <utility>
#include <vector>

#include "core/ExceptionPACS.h"
#include "core/LoggerPACS.h"
#include "MoveSCP.h"
#include "services/ServicesTools.h"
#include "services/SCP/PresentationContext.h"

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

    Uint16 status = STATUS_Pending;

    if (responseCount == 1)
    {
        requestIdentifiers->insertEmptyElement(DCM_TransferSyntaxUID);
        status = context->get_generator()->process_dataset(
            requestIdentifiers, false);

        if (status != STATUS_Pending)
        {
            create_status_detail(
                    status, DCM_UndefinedTagKey,
                    OFString("An error occured while processing Move operation"),
                    stDetail);
        }

        // Find all presentation contexts from the data sets
        auto & generator = *context->get_generator();
        std::remove_reference<decltype(generator)>::type pc_generator(
            generator.get_username());
        DcmDataset pc_query = *requestIdentifiers;
        pc_query.insertEmptyElement(DCM_TransferSyntaxUID);
        pc_query.insertEmptyElement(DCM_SOPClassUID);
        status = pc_generator.process_dataset(&pc_query, false);

        std::set<std::pair<std::string, std::string>> syntax_pairs;

        mongo::BSONObj item;
        while(!(item = pc_generator.next()).isEmpty())
        {
            auto const abstract_syntax = item["00080016"]["Value"].Array()[0].String();
            auto const transfer_syntax = item["00020010"]["Value"].Array()[0].String();
            syntax_pairs.insert(std::make_pair(abstract_syntax, transfer_syntax));
        }

        std::vector<PresentationContext> presentation_contexts;
        for(auto const & syntax_pair: syntax_pairs)
        {
            PresentationContext presentation_context({
                syntax_pair.first, {syntax_pair.second}, ASC_SC_ROLE_SCU});
            presentation_contexts.push_back(presentation_context);
        }

        // Create Move SubAssociation
        OFCondition condition =
            context->get_storeprovider()->build_sub_association(
                request->MoveDestination, presentation_contexts);
        if (condition.bad())
        {
            dopamine::logger_error() << "Cannot create sub association: "
                                    << condition.text();
            create_status_detail(0xc000, DCM_UndefinedTagKey,
                                 OFString(condition.text()), stDetail);

            status = 0xc000; // Unable to process
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
            std::string const transfer_syntax =
                object["00020010"]["Value"].Array()[0].String();

            auto * const data_set =
                context->get_generator()->retrieve_dataset(object);

            OFCondition condition =
                context->get_storeprovider()->perform_sub_operation(
                    data_set, transfer_syntax, request->Priority);

            delete data_set;

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
