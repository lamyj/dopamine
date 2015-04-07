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
static void subProcessCallback(void*, T_DIMSE_StoreProgress * progress,
                               T_DIMSE_C_StoreRQ* request)
{
    // Nothing to do
}

StoreSubOperation
::StoreSubOperation(T_ASC_Network * network,
                    T_ASC_Association * request_association,
                    DIC_US messageID):
    _network(network), _request_association(request_association),
    _response_association(request_association), _originalMsgID(messageID),
    _new_association(false)
{
    this->_originalAETitle[0] = '\0';
}

StoreSubOperation
::~StoreSubOperation()
{
    if (this->_new_association && this->_response_association != NULL)
    {
        OFCondition cond = ASC_releaseAssociation(this->_response_association);
        if (cond.bad())
        {
            OFString temp_str;
            dopamine::loggerError() << "Cannot Release Association: "
                                         << DimseCondition::dump(temp_str, cond);
        }
        cond = ASC_destroyAssociation(&this->_response_association);
        if (cond.bad())
        {
            OFString temp_str;
            dopamine::loggerError() << "Cannot Destroy Association: "
                                         << DimseCondition::dump(temp_str, cond);
        }
    }
}

OFCondition
StoreSubOperation
::buildSubAssociation(DIC_AE destinationAETitle)
{
    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(this->_request_association->params,
                    this->_originalAETitle, aeTitle, NULL);

    std::string dstHostNamePort;
    if (!ConfigurationPACS::get_instance().peerForAETitle(std::string(destinationAETitle),
                                                          dstHostNamePort))
    {
        dopamine::loggerError() << "Invalid Peer for move operation";
        return EC_IllegalParameter;
    }

    T_ASC_Parameters* params;
    OFCondition result = ASC_createAssociationParameters(&params,
                                                         ASC_DEFAULTMAXPDU);
    if (result.bad())
    {
        return result;
    }

    DIC_NODENAME localHostName;
    gethostname(localHostName, sizeof(localHostName) - 1);

    ASC_setPresentationAddresses(params, localHostName, dstHostNamePort.c_str());

    ASC_setAPTitles(params, aeTitle, destinationAETitle, NULL);

    result = this->addAllStoragePresentationContext(params);
    if (result.bad())
    {
        return result;
    }

    // Create Association
    result = ASC_requestAssociation(this->_network, params, &this->_response_association);

    if(result.bad())
    {
        OFString empty;

        if(result == DUL_ASSOCIATIONREJECTED)
        {
            T_ASC_RejectParameters rej;
            ASC_getRejectParameters(params, &rej);

            dopamine::loggerError() << ASC_printRejectParameters(empty, &rej).c_str();
        }
        else
        {
            dopamine::loggerError() << DimseCondition::dump(empty, result).c_str();
        }
    }

    this->_new_association = true;

    return result;
}

OFCondition
StoreSubOperation
::performSubOperation(DcmDataset *dataset, T_DIMSE_Priority priority)
{
    OFString sopclassuid;
    OFCondition result = dataset->findAndGetOFString(DCM_SOPClassUID,
                                                     sopclassuid);
    if (result.bad())
    {
        dopamine::loggerError() << "Cannot retrieve SOPClassUID from dataset: "
                                << result.text();
        return result;
    }
    OFString sopinstanceuid;
    result = dataset->findAndGetOFString(DCM_SOPInstanceUID,
                                         sopinstanceuid);
    if (result.bad())
    {
        dopamine::loggerError() << "Cannot retrieve SOPInstanceUID from dataset: "
                                << result.text();
        return result;
    }

    /* which presentation context should be used */
    T_ASC_PresentationContextID presID;
    presID = ASC_findAcceptedPresentationContextID(this->_response_association,
                                                   sopclassuid.c_str());
    if (presID == 0)
    {
        dopamine::loggerError() << "No presentation context for: "
                                << dcmSOPClassUIDToModality(sopclassuid.c_str(), "OT");
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }
    else if (!this->_new_association)
    {
        /* make sure that we can send images in this presentation context */
        T_ASC_PresentationContext pc;
        OFCondition condition =
                ASC_findAcceptedPresentationContext(this->_response_association->params,
                                                    presID, &pc);
        if (condition.bad())
        {
            return condition;
        }

        if (pc.acceptedRole != ASC_SC_ROLE_SCP &&
            pc.acceptedRole != ASC_SC_ROLE_SCUSCP)
        {
            dopamine::loggerError() << "No presentation context with requestor SCP role for: "
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
        strcpy(req.MoveOriginatorApplicationEntityTitle, this->_originalAETitle);
        req.MoveOriginatorID = this->_originalMsgID;
    }

    dopamine::loggerInfo() << "Store SCU RQ: MsgID " << msgID;

    T_DIMSE_DetectedCancelParameters cancelParameters;
    T_DIMSE_C_StoreRSP rsp;
    DcmDataset* stdetail = NULL;
    return DIMSE_storeUser(this->_response_association, presID, &req, NULL,
                           dataset, subProcessCallback, this,
                           DIMSE_BLOCKING, 0, &rsp, &stdetail,
                           &cancelParameters);
}

OFCondition
StoreSubOperation
::addAllStoragePresentationContext(T_ASC_Parameters *params)
{
    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };
    int numTransferSyntaxes = 0;
    if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
    {
      transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
    } else {
      transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
    }
    transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
    numTransferSyntaxes = 3;

    OFCondition cond = EC_Normal;
    int pid = 1;
    for (int i = 0; i < numberOfDcmLongSCUStorageSOPClassUIDs && cond.good(); i++)
    {
        cond = ASC_addPresentationContext(params, pid, dcmLongSCUStorageSOPClassUIDs[i],
                                          transferSyntaxes, numTransferSyntaxes);
        pid += 2;
    }

    return cond;
}

} // namespace services

} // namespace dopamine
