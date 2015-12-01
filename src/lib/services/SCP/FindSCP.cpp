/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/conversion.h>
#include <dcmtkpp/registry.h>
#include <dcmtkpp/message/Response.h>
#include <dcmtkpp/Tag.h>

#include "core/LoggerPACS.h"
#include "FindSCP.h"
#include "services/QueryGenerator.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace services
{

/**
 * Callback handler called by the DIMSE_findProvider callback function
 * @param callbackdata: Callback context (in)
 * @param cancelled: flag indicating wheter a C-CANCEL was received (in)
 * @param request: original find request (in)
 * @param requestIdentifiers: original find request identifiers (in)
 * @param responseCount: find response count (in)
 * @param response: final find response (out)
 * @param responseIdentifiers: find response identifiers (out)
 * @param stDetail: status detail for find response (out)
 */
static void find_callback(
        /* in */
        void *callbackData, OFBool cancelled, T_DIMSE_C_FindRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_FindRSP *response, DcmDataset **responseIdentifiers,
        DcmDataset **stDetail)
{
    QueryGenerator* context =
            reinterpret_cast<QueryGenerator*>(callbackData);

    Uint16 status = dcmtkpp::message::Response::Pending;
    dcmtkpp::DataSet details;

    if (responseCount == 1)
    {
        status = context->process_dataset(dcmtkpp::convert(requestIdentifiers), false);
        if (status != dcmtkpp::message::Response::Pending)
        {
            details = create_status_detail(status, dcmtkpp::Tag(0xffff, 0xffff),
                                           "An error occured while processing Find operation");
        }
    }

    /* only cancel if we have pending responses */
    if (cancelled && status == dcmtkpp::message::Response::Pending)
    {
        context->cancel();
    }

    /* Process next result */
    if (status == dcmtkpp::message::Response::Pending)
    {
        mongo::BSONObj object = context->next();

        if (object.isValid() && object.isEmpty())
        {
            // We're done.
            status = dcmtkpp::message::Response::Success;
        }
        else if (object.hasField("$err"))
        {
            dopamine::logger_error()
                    << "An error occured while processing Find operation: "
                    << object.getField("$err").String();

            status = STATUS_FIND_Failed_UnableToProcess;

            details = create_status_detail(0xc000, dcmtkpp::Tag(0xffff, 0xffff),
                                           object["$err"].String());
        }
        else
        {
            dcmtkpp::DataSet data_set = context->retrieve_dataset(object);

            data_set.add(dcmtkpp::registry::QueryRetrieveLevel, dcmtkpp::Element({context->get_query_retrieve_level()}, dcmtkpp::VR::CS));

            if (status == dcmtkpp::message::Response::Pending && object.hasField("instance_count"))
            {
                /*OFString count(12, '\0');
                snprintf(&count[0], 12, "%i",
                         int(object.getField("instance_count").Number()));
                condition =
                        (*responseIdentifiers)->putAndInsertOFStringArray(
                            context->get_instance_count_tag(), count);

                if (condition.bad())
                {
                    dopamine::logger_error()
                            << "Cannot insert "
                            << context->get_instance_count_tag().getGroup()
                            << ","
                            << context->get_instance_count_tag().getElement()
                            << ": " << condition .text();

                    status = STATUS_FIND_Failed_UnableToProcess;

                    create_status_detail(STATUS_FIND_Failed_UnableToProcess,
                                       context->get_instance_count_tag(),
                                       OFString(condition.text()), stDetail);
                }*/
            }

            if (status == dcmtkpp::message::Response::Pending &&
                context->get_convert_modalities_in_study())
            {
                data_set.remove(dcmtkpp::registry::Modality);
                std::vector<mongo::BSONElement> const modalities =
                        object.getField("modalities_in_study").Array();
                dcmtkpp::Value::Strings values;
                for(unsigned int i=0; i<modalities.size(); ++i)
                {
                    values.push_back(modalities[i].String());
                }
                data_set.add(dcmtkpp::registry::ModalitiesInStudy, dcmtkpp::Element(values, dcmtkpp::VR::CS));
            }

            (*responseIdentifiers) = dynamic_cast<DcmDataset*>(dcmtkpp::convert(data_set, true));
        }
    }

    /* set response status */
    response->DimseStatus = status;
    if (status == dcmtkpp::message::Response::Pending ||
        status == dcmtkpp::message::Response::Success)
    {
        *stDetail = NULL;
    }
    else
    {
        DcmItem* item = dcmtkpp::convert(details, true);
        DcmDataset* dcmdtset = dynamic_cast<DcmDataset*>(item);
        (*stDetail) = new DcmDataset(*dcmdtset);
    }
}
    
FindSCP
::FindSCP(T_ASC_Association * association,
          T_ASC_PresentationContextID presentation_context_id,
          T_DIMSE_C_FindRQ * request):
    SCP(association, presentation_context_id), // base class initialisation
    _request(request)
{
    // nothing to do
}

FindSCP
::~FindSCP()
{
    // nothing to do
}

OFCondition
FindSCP
::process()
{
    dopamine::logger_info() << "Received Find SCP: MsgID "
                                << this->_request->MessageID;

    std::string const username =
           get_username(this->_association->params->DULparams.reqUserIdentNeg);
    QueryGenerator context(username);
    
    return DIMSE_findProvider(this->_association, this->_presentation_context_id,
                              this->_request, find_callback, &context,
                              DIMSE_BLOCKING, 30);
}

} // namespace services

} // namespace dopamine
