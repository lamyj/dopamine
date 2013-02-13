#include "GenericSCP.h"

#include <stdio.h>

#include <dcmtk/dcmnet/diutil.h>
#include <dcmtk/dcmnet/scp.h>
#include <gdcmReader.h>
#include <mongo/bson/bson.h>

#include "DataSetToBSON.h"

GenericSCP
::~GenericSCP()
{

}

template<>
OFCondition
GenericSCP
::handleCommand_<DIMSE_C_ECHO_RQ>(T_DIMSE_Message * message,
                                  DcmPresentationContextInfo const & info)
{
    return this->handleECHORequest(message->msg.CEchoRQ, info.presentationContextID);
}

template<>
OFCondition
GenericSCP
::handleCommand_<DIMSE_C_STORE_RQ>(T_DIMSE_Message * message,
                                   DcmPresentationContextInfo const & info)
{
    // cf. http://forum.dcmtk.org/viewtopic.php?f=1&t=3213&sid=b5fc0c5531c286df56b7e672bb333dc4&start=60

    OFCondition cond = DIMSE_BADCOMMANDTYPE;

    T_ASC_PresentationContextID presentation_id = info.presentationContextID;
    T_DIMSE_C_StoreRQ request = message->msg.CStoreRQ;
    T_DIMSE_C_StoreRSP response;

    DcmDataset * dataset = NULL;
    if(this->receiveDIMSEDataset(&presentation_id, &dataset,NULL,NULL).good())
    {
        if(dataset != NULL)
        {
            // Save the DCMTK dataset to a temporary file
            char * filename = tempnam(NULL, NULL);
            dataset->saveFile(filename);
            delete dataset;

            // Re-load it as a GDCM dataset
            gdcm::Reader reader;
            reader.SetFileName(filename);
            reader.Read();

            // TODO : store this->getPeerAETitle() in Source Application Entity Title
            // Convert the GDCM dataset to BSON
            DataSetToBSON converter;
            mongo::BSONObjBuilder builder;
            converter(reader.GetFile().GetDataSet(), builder);
            mongo::BSONObj const bson = builder.obj();

            // Store it in the Mongo DB instance
            //std::cout << "Converted (" << bson.getField("00100010") << ")" << std::endl;

            // Clean up the temporary file
            unlink(filename);
            free(filename);

            // Generate the DICOM response
            bzero((char*)&response, sizeof(response));
            response.DimseStatus = STATUS_Success;
            response.MessageIDBeingRespondedTo = request.MessageID;
            response.DataSetType = DIMSE_DATASET_NULL;
            strcpy(response.AffectedSOPClassUID, request.AffectedSOPClassUID);
            strcpy(response.AffectedSOPInstanceUID, request.AffectedSOPInstanceUID);
            response.opts = (O_STORE_AFFECTEDSOPCLASSUID | O_STORE_AFFECTEDSOPINSTANCEUID);
            if(request.opts & O_STORE_RQ_BLANK_PADDING)
            {
                response.opts |= O_STORE_RSP_BLANK_PADDING;
            }
            if (dcmPeerRequiresExactUIDCopy.get())
            {
                response.opts |= O_STORE_PEER_REQUIRES_EXACT_UID_COPY;
            }

            DcmDataset *statusDetail = NULL;
            cond = this->sendSTOREResponse(presentation_id, request, response, statusDetail);
            if(cond.good())
            {
                DCMNET_INFO("C-STORE Response successfully sent");
            }
            else
            {
                DCMNET_INFO("Cannot send C-Store Response: " );
            }
        }
        else
        {
            std::cout << "No dataset" << std::endl;
            // TODO : value of cond ?
        }
    }
    else
    {
        std::cout << "No data received" << std::endl;
        // TODO : value of cond ?
    }

    return cond;
}

OFCondition
GenericSCP
::handleIncomingCommand(T_DIMSE_Message * msg,
                        DcmPresentationContextInfo const & info)
{
    OFCondition cond;
    if(msg->CommandField == DIMSE_C_ECHO_RQ)
    {
        cond = this->handleCommand_<DIMSE_C_ECHO_RQ>(msg, info);
    }
    else if(msg->CommandField == DIMSE_C_STORE_RQ)
    {
        cond = this->handleCommand_<DIMSE_C_STORE_RQ>(msg, info);
    }
    else
    {
        cond = this->handleCommand_<DIMSE_NOTHING>(msg, info);
    }

    // return result
    return cond;
}
