/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "core/ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "StoreSubOperation.h"
#include "ServicesTools.h"

namespace dopamine
{

namespace services
{

/**
 * Callback handler called by the DIMSE_storeProvider callback function
 * @param progress: progress state (in)
 * @param request: original store request (in)
 */
static void sub_process_callback(void*, T_DIMSE_StoreProgress * progress,
                                 T_DIMSE_C_StoreRQ* request)
{
    // Nothing to do
}

StoreSubOperation
::StoreSubOperation(T_ASC_Network * network,
                    T_ASC_Association * request_association,
                    DIC_US messageID):
    _network(network), _request_association(request_association),
    _response_association(request_association), _original_message_id(messageID),
    _new_association(false)
{
    this->_original_aetitle[0] = '\0';
}

StoreSubOperation
::~StoreSubOperation()
{
    if (this->_new_association && this->_response_association != NULL)
    {
        OFCondition condition =
                ASC_releaseAssociation(this->_response_association);
        if (condition.bad())
        {
            OFString temp_str;
            dopamine::logger_error() << "Cannot Release Association: "
                                     << DimseCondition::dump(temp_str,
                                                             condition);
        }
        condition = ASC_destroyAssociation(&this->_response_association);
        if (condition.bad())
        {
            OFString temp_str;
            dopamine::logger_error() << "Cannot Destroy Association: "
                                     << DimseCondition::dump(temp_str,
                                                             condition);
        }
    }
}

OFCondition
StoreSubOperation
::build_sub_association(DIC_AE destination_aetitle)
{
    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(this->_request_association->params,
                    this->_original_aetitle, aeTitle, NULL);

    std::string dstHostNamePort;
    if (!ConfigurationPACS::
            get_instance().peer_for_aetitle(std::string(destination_aetitle),
                                            dstHostNamePort))
    {
        dopamine::logger_error() << "Invalid Peer for move operation";
        return EC_IllegalParameter;
    }

    T_ASC_Parameters* params;
    OFCondition condition = ASC_createAssociationParameters(&params,
                                                            ASC_DEFAULTMAXPDU);
    if (condition.bad())
    {
        return condition;
    }

    DIC_NODENAME localHostName;
    gethostname(localHostName, sizeof(localHostName) - 1);

    condition = ASC_setPresentationAddresses(params,
                                             localHostName,
                                             dstHostNamePort.c_str());
    if (condition.bad())
    {
        return condition;
    }

    condition = ASC_setAPTitles(params, aeTitle, destination_aetitle, NULL);
    if (condition.bad())
    {
        return condition;
    }

    condition = this->_add_all_storage_presentation_context(params);
    if (condition.bad())
    {
        return condition;
    }

    // Create Association
    condition = ASC_requestAssociation(this->_network,
                                       params,
                                       &this->_response_association);

    if(condition.bad())
    {
        OFString empty;

        if(condition == DUL_ASSOCIATIONREJECTED)
        {
            T_ASC_RejectParameters rej;
            ASC_getRejectParameters(params, &rej);

            dopamine::logger_error()
                    << ASC_printRejectParameters(empty, &rej).c_str();
        }
        else
        {
            dopamine::logger_error()
                    << DimseCondition::dump(empty, condition).c_str();
        }
    }

    this->_new_association = true;

    return condition;
}

OFCondition
StoreSubOperation
::perform_sub_operation(DcmDataset *dataset, T_DIMSE_Priority priority)
{
    OFString sopclassuid;
    OFCondition condition = dataset->findAndGetOFString(DCM_SOPClassUID,
                                                        sopclassuid);
    if (condition.bad())
    {
        dopamine::logger_error() << "Cannot retrieve SOPClassUID from dataset: "
                                 << condition.text();
        return condition;
    }
    OFString sopinstanceuid;
    condition = dataset->findAndGetOFString(DCM_SOPInstanceUID,
                                            sopinstanceuid);
    if (condition.bad())
    {
        dopamine::logger_error()
                << "Cannot retrieve SOPInstanceUID from dataset: "
                << condition.text();
        return condition;
    }

    /* which presentation context should be used */
    T_ASC_PresentationContextID presentation_id =
            ASC_findAcceptedPresentationContextID(this->_response_association,
                                                  sopclassuid.c_str());
    if (presentation_id == 0)
    {
        dopamine::logger_error() << "No presentation context for: "
                                 << dcmSOPClassUIDToModality(sopclassuid.c_str(),
                                                             "OT");
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }
    else if (!this->_new_association)
    {
        /* make sure that we can send images in this presentation context */
        T_ASC_PresentationContext pc;
        OFCondition condition =
                ASC_findAcceptedPresentationContext(
                    this->_response_association->params,
                    presentation_id, &pc);
        if (condition.bad())
        {
            return condition;
        }

        if (pc.acceptedRole != ASC_SC_ROLE_SCP &&
            pc.acceptedRole != ASC_SC_ROLE_SCUSCP)
        {
            dopamine::logger_error()
                    << "No presentation context with requestor SCP role for: "
                    << dcmSOPClassUIDToModality(sopclassuid.c_str(), "OT");
            return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
        }
    }

    DIC_US msgID = this->_response_association->nextMsgID++;

    T_DIMSE_C_StoreRQ req;
    req.MessageID = msgID;
    strcpy(req.AffectedSOPClassUID, sopclassuid.c_str());
    strcpy(req.AffectedSOPInstanceUID, sopinstanceuid.c_str());
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = priority;
    if (this->_new_association)
    {
        req.opts = O_STORE_MOVEORIGINATORAETITLE | O_STORE_MOVEORIGINATORID;
        strcpy(req.MoveOriginatorApplicationEntityTitle,
               this->_original_aetitle);
        req.MoveOriginatorID = this->_original_message_id;
    }

    dopamine::logger_info() << "Store SCU RQ: MsgID " << msgID;

    T_DIMSE_DetectedCancelParameters cancelParameters;
    T_DIMSE_C_StoreRSP rsp;
    DcmDataset* stdetail = NULL;
    return DIMSE_storeUser(this->_response_association, presentation_id,
                           &req, NULL, dataset, sub_process_callback,
                           this, DIMSE_BLOCKING, 0, &rsp, &stdetail,
                           &cancelParameters);
}

OFCondition
StoreSubOperation
::_add_all_storage_presentation_context(T_ASC_Parameters *params)
{
    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };
    int numTransferSyntaxes = 0;
    if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
    {
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
    }
    else
    {
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
    }
    transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
    numTransferSyntaxes = 3;

    OFCondition condition = EC_Normal;
    int pid = 1;
    for (int i = 0;
         i < numberOfDcmLongSCUStorageSOPClassUIDs && condition.good(); i++)
    {
        condition =
                ASC_addPresentationContext(params, pid,
                                           dcmLongSCUStorageSOPClassUIDs[i],
                                           transferSyntaxes,
                                           numTransferSyntaxes);
        pid += 2;
    }

    return condition;
}

} // namespace services

} // namespace dopamine
