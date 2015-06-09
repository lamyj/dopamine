/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

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

    Uint16 status = STATUS_Pending;

    if (responseCount == 1)
    {
        status = context->process_dataset(requestIdentifiers, false);
        if (status != STATUS_Pending)
        {
            create_status_detail(status, DCM_UndefinedTagKey,
                                 OFString("An error occured while processing Find operation"),
                                 stDetail);
        }
    }

    /* only cancel if we have pending responses */
    if (cancelled && status == STATUS_Pending)
    {
        context->cancel();
    }

    /* Process next result */
    if (status == STATUS_Pending)
    {
        mongo::BSONObj object = context->next();

        if (object.isValid() && object.isEmpty())
        {
            // We're done.
            status = STATUS_Success;
        }
        else if (object.hasField("$err"))
        {
            dopamine::logger_error() << "An error occured while processing Find operation: "
                                     << object.getField("$err").String();

            status = STATUS_FIND_Failed_UnableToProcess;

            create_status_detail(STATUS_FIND_Failed_UnableToProcess,
                                 DCM_UndefinedTagKey,
                                 OFString(object.getField("$err").String().c_str()), stDetail);
        }
        else
        {
            (*responseIdentifiers) = context->retrieve_dataset(object);

            OFCondition condition =
                    (*responseIdentifiers)->putAndInsertOFStringArray(DCM_QueryRetrieveLevel,
                                              context->get_query_retrieve_level().c_str());

            if (condition.bad())
            {
                dopamine::logger_error() << "Cannot insert DCM_QueryRetrieveLevel: "
                                         << condition .text();

                status = STATUS_FIND_Failed_UnableToProcess;

                create_status_detail(STATUS_FIND_Failed_UnableToProcess,
                                     DCM_QueryRetrieveLevel,
                                     OFString(condition.text()), stDetail);
            }

            if (status == STATUS_Pending && object.hasField("instance_count"))
            {
                /*OFString count(12, '\0');
                snprintf(&count[0], 12, "%i", int(object.getField("instance_count").Number()));
                condition =
                        (*responseIdentifiers)->putAndInsertOFStringArray(context->get_instance_count_tag(),
                                                                          count);

                if (condition.bad())
                {
                    dopamine::logger_error() << "Cannot insert "
                                             << context->get_instance_count_tag().getGroup() << ","
                                             << context->get_instance_count_tag().getElement() << ": "
                                             << condition .text();

                    status = STATUS_FIND_Failed_UnableToProcess;

                    create_status_detail(STATUS_FIND_Failed_UnableToProcess,
                                       context->get_instance_count_tag(),
                                       OFString(condition.text()), stDetail);
                }*/
            }

            if (status == STATUS_Pending && context->get_convert_modalities_in_study())
            {
                (*responseIdentifiers)->remove(DCM_Modality);
                std::vector<mongo::BSONElement> const modalities =
                        object.getField("modalities_in_study").Array();
                std::string value;
                for(unsigned int i=0; i<modalities.size(); ++i)
                {
                    value += modalities[i].String();
                    if(i!=modalities.size()-1)
                    {
                        value += "\\";
                    }
                }
                condition = (*responseIdentifiers)->putAndInsertOFStringArray(DCM_ModalitiesInStudy,
                                                                              OFString(value.c_str()));

                if (condition.bad())
                {
                    dopamine::logger_error() << "Cannot insert DCM_ModalitiesInStudy: "
                                             << condition .text();

                    status = STATUS_FIND_Failed_UnableToProcess;

                    create_status_detail(STATUS_FIND_Failed_UnableToProcess,
                                         DCM_ModalitiesInStudy,
                                         OFString(condition.text()), stDetail);
                }
            }
        }
    }

    /* set response status */
    response->DimseStatus = status;
    if (status == STATUS_Pending || status == STATUS_Success)
    {
        *stDetail = NULL;
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
