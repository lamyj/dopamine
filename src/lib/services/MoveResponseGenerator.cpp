/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "ConverterBSON/BSONToDataSet.h"
#include "ConverterBSON/DataSetToBSON.h"
#include "ConverterBSON/TagMatch.h"
#include "core/ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "MoveResponseGenerator.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace services
{
    
/**
 * Callback handler called by the DIMSE_storeProvider callback function
 * @param progress: progress state (in)
 * @param request: original store request (in)
 */
static void moveSubProcessCallback(void*, T_DIMSE_StoreProgress * progress,
                                   T_DIMSE_C_StoreRQ*)
{
    // Nothing to do
}
    
MoveResponseGenerator
::MoveResponseGenerator(T_ASC_Association *request_association):
    ResponseGenerator(request_association, Service_Retrieve), // base class initialisation
    _priority(DIMSE_PRIORITY_MEDIUM)
{
    _origAETitle[0] = '\0';
}

MoveResponseGenerator
::~MoveResponseGenerator()
{
    //Nothing to do
}

void 
MoveResponseGenerator
::process(
        /* in */
        OFBool cancelled, T_DIMSE_C_MoveRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_MoveRSP* response, DcmDataset** stDetail,
        DcmDataset** responseIdentifiers)
{
    if (responseCount == 1)
    {
        this->_status = this->set_query(requestIdentifiers);

        if (this->_status != STATUS_Success && this->_status != STATUS_Pending)
        {
            response->DimseStatus = this->_status;
            createStatusDetail(this->_status, DCM_UndefinedTagKey,
                               OFString("An error occured while processing Move operation"),
                               stDetail);
        }

        _origMsgID = request->MessageID;
        
        // Build a new association to the move destination
        OFCondition condition = this->buildSubAssociation(request, stDetail);
        if (condition.bad())
        {
            dopamine::loggerError() << "Cannot create sub association: "
                                    << condition.text();
        }
    } // if (responseCount == 1)
    
    /* only cancel if we have pending responses */
    if (cancelled && this->_status == STATUS_Pending)
    {
        this->cancel();
    }
    
    /* Process next result */
    if (this->_status == STATUS_Pending)
    {
        this->_priority = request->Priority;
        this->next(responseIdentifiers, stDetail);
    }
    
    /* set response status */
    response->DimseStatus = this->_status;
    if (this->_status == STATUS_Pending || this->_status == STATUS_Success)
    {
        (*stDetail) = NULL;
    }
}

void
MoveResponseGenerator
::set_network(T_ASC_Network *network)
{
    this->_network = network;
}

void 
MoveResponseGenerator
::next(DcmDataset ** responseIdentifiers, DcmDataset **details)
{
    if(!this->_cursor->more())
    {
        // We're done.
        this->_status = STATUS_Success;

        OFCondition cond = ASC_releaseAssociation(this->_subAssociation);
        if (cond.bad())
        {
            OFString temp_str;
            dopamine::loggerError() << "Cannot Release Association: "
                                         << DimseCondition::dump(temp_str, cond);
        }
        cond = ASC_destroyAssociation(&this->_subAssociation);
        if (cond.bad())
        {
            OFString temp_str;
            dopamine::loggerError() << "Cannot Destroy Association: "
                                         << DimseCondition::dump(temp_str, cond);
        }
    }
    else
    {
        mongo::BSONObj item = this->_cursor->next();
        
        if ( ! item.hasField("location"))
        {
            dopamine::loggerError() << "Unable to retrieve location field";

            createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                               DCM_UndefinedTagKey, OFString(EC_CorruptedData.text()), details);

            this->_status = STATUS_MOVE_Failed_UnableToProcess;
            return;
        }
        
        std::string const path = item.getField("location").String();
        DcmFileFormat fileformat;
        OFCondition result = fileformat.loadFile(path.c_str());
        if (result.bad())
        {
            dopamine::loggerError() << "Cannot load dataset " << path << " : "
                                    << result.text();

            createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                               DCM_UndefinedTagKey, OFString(result.text()), details);

            this->_status = STATUS_MOVE_Failed_UnableToProcess;
            return;
        }
        DcmDataset* dataset = fileformat.getAndRemoveDataset();
        
        OFString sopclassuid;
        result = dataset->findAndGetOFString(DCM_SOPClassUID,
                                                         sopclassuid);
        if (result.bad())
        {
            dopamine::loggerError() << "Cannot retrieve SOPClassUID in dataset: "
                                    << result.text();

            createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                               DCM_SOPClassUID, OFString(result.text()), details);

            this->_status = STATUS_MOVE_Failed_UnableToProcess;
            return;
        }
        OFString sopinstanceuid;
        result = dataset->findAndGetOFString(DCM_SOPInstanceUID, 
                                             sopinstanceuid);
        if (result.bad())
        {
            dopamine::loggerError() << "Cannot retrieve SOPInstanceUID in dataset: "
                                    << result.text();

            createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                               DCM_SOPInstanceUID, OFString(result.text()), details);

            this->_status = STATUS_MOVE_Failed_UnableToProcess;
            return;
        }
        
        // Perform sub operation
        result = this->performMoveSubOperation(sopclassuid.c_str(), 
                                               sopinstanceuid.c_str(), 
                                               dataset);
        if (result.bad())
        {
            dopamine::loggerError() << "Move Sub-Op Failed: " << result.text();

            createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                               DCM_UndefinedTagKey, OFString(result.text()), details);

            this->_status = STATUS_MOVE_Failed_UnableToProcess;

            return;
        }

        this->_status = STATUS_Pending;
    }
}

OFCondition 
MoveResponseGenerator
::performMoveSubOperation(const char* sopClassUID, 
                          const char* sopInstanceUID,
                          DcmDataset* dataset)
{
    /* which presentation context should be used */
    T_ASC_PresentationContextID presID;
    presID = ASC_findAcceptedPresentationContextID(this->_subAssociation, 
                                                   sopClassUID);
    if (presID == 0)
    {
        dopamine::loggerError() << "No presentation context for: "
                                << dcmSOPClassUIDToModality(sopClassUID, "OT");
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }
    
    DIC_US msgID = this->_subAssociation->nextMsgID++;
    
    T_DIMSE_C_StoreRQ req;
    req.MessageID = msgID;
    strcpy(req.AffectedSOPClassUID, sopClassUID);
    strcpy(req.AffectedSOPInstanceUID, sopInstanceUID);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = this->_priority;
    req.opts = O_STORE_MOVEORIGINATORAETITLE | O_STORE_MOVEORIGINATORID;
    strcpy(req.MoveOriginatorApplicationEntityTitle, this->_origAETitle);
    req.MoveOriginatorID = this->_origMsgID;
    
    dopamine::loggerInfo() << "Store SCU RQ: MsgID " << msgID;
    
    T_DIMSE_DetectedCancelParameters cancelParameters;
    T_DIMSE_C_StoreRSP rsp;
    DcmDataset* stdetail = NULL;
    return DIMSE_storeUser(this->_subAssociation, presID, &req, NULL,
                           dataset, moveSubProcessCallback, this,
                           DIMSE_BLOCKING, 0, &rsp, &stdetail,
                           &cancelParameters);
}

OFCondition 
MoveResponseGenerator
::buildSubAssociation(T_DIMSE_C_MoveRQ* request, DcmDataset **details)
{
    DIC_AE dstAETitle;
    dstAETitle[0] = '\0';
    
    strcpy(dstAETitle, request->MoveDestination);
    
    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(this->_request_association->params,
                    this->_origAETitle, aeTitle, NULL);
    
    std::string dstHostNamePort;
    if (!ConfigurationPACS::get_instance().peerForAETitle(std::string(request->MoveDestination), 
                                                          dstHostNamePort))
    {
        dopamine::loggerError() << "Invalid Peer for move operation";

        createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                           DCM_UndefinedTagKey, OFString(EC_IllegalParameter.text()), details);

        this->_status = STATUS_MOVE_Failed_UnableToProcess;
        return EC_IllegalParameter;
    }

    T_ASC_Parameters* params;
    OFCondition result = ASC_createAssociationParameters(&params, 
                                                         ASC_DEFAULTMAXPDU);
    if (result.bad())
    {
        createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                           DCM_UndefinedTagKey, OFString(result.text()), details);

        this->_status = STATUS_MOVE_Failed_UnableToProcess;
        return result;
    }
    
    DIC_NODENAME localHostName;
    gethostname(localHostName, sizeof(localHostName) - 1);
    
    ASC_setPresentationAddresses(params, localHostName, dstHostNamePort.c_str());
    
    ASC_setAPTitles(params, this->_ourAETitle.c_str(), dstAETitle, NULL);
    
    result = this->addAllStoragePresentationContext(params);
    if (result.bad())
    {
        createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                           DCM_UndefinedTagKey, OFString(result.text()), details);

        this->_status = STATUS_MOVE_Failed_UnableToProcess;
        return result;
    }
    
    // Create Association
    result = ASC_requestAssociation(this->_network, params, &this->_subAssociation);
    
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

        createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                           DCM_UndefinedTagKey, OFString(result.text()), details);

        this->_status = STATUS_MOVE_Failed_UnableToProcess;
        return result;
    }

    return result;
}

OFCondition 
MoveResponseGenerator
::addAllStoragePresentationContext(T_ASC_Parameters* params)
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
