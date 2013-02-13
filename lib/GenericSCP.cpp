#include "GenericSCP.h"

#include <sstream>
#include <stdio.h>
#include <string>

#include <dcmtk/dcmdata/dcostrmb.h>
#include <dcmtk/dcmnet/diutil.h>
#include <dcmtk/dcmnet/scp.h>
#include <gdcmReader.h>
#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

#include "DataSetToBSON.h"

GenericSCP
::GenericSCP(std::string const & db_name,
             std::string const & host, unsigned int port)
: _db_name(db_name), _grid_fs(NULL)
{
    std::stringstream stream;
    stream << port;
    this->_connection.connect(host+":"+stream.str());
    this->_grid_fs = new mongo::GridFS(this->_connection, this->_db_name);
}

GenericSCP
::~GenericSCP()
{
    if(this->_grid_fs != NULL)
    {
        delete this->_grid_fs;
    }
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
    if(this->receiveDIMSEDataset(&presentation_id, &dataset, NULL, NULL).good())
    {
        if(dataset != NULL)
        {
            // Check if we already have this dataset, based on its SOP Instance UID
            OFString sop_instance_uid;
            dataset->findAndGetOFString(DcmTagKey(0x0008,0x0018), sop_instance_uid);

            mongo::auto_ptr<mongo::DBClientCursor> cursor =
                this->_connection.query(this->_db_name+"."+"datasets",
                                        QUERY("00080018" << sop_instance_uid.c_str()));
            if(cursor->more())
            {
                // We already have this SOP Instance UID, do not store it
            }
            else
            {
                // Save the DCMTK dataset to a temporary file
                char * filename = tempnam(NULL, NULL);
                dataset->saveFile(filename,
                    DcmXfer(info.acceptedTransferSyntax.c_str()).getXfer()
                    // TODO : encoding type, group length, encoding, ...
                );
                delete dataset;

                // Re-load it as a GDCM dataset
                gdcm::Reader reader;
                reader.SetFileName(filename);
                reader.Read();

                // TODO : store this->getPeerAETitle() in Source Application Entity Title
                // Convert the GDCM dataset to BSON
                DataSetToBSON converter;
                converter.set_filter(DataSetToBSON::Filter::EXCLUDE);
                converter.add_filtered_tag(0x7fe00010);
                mongo::BSONObjBuilder builder;
                converter(reader.GetFile().GetDataSet(), builder);

                // Store it in the Mongo DB instance
                mongo::OID const id(mongo::OID::gen());
                builder << "_id" << id;
                // Warning: You must call OID::newState() after a fork().

                this->_connection.insert(this->_db_name+".datasets", builder.obj());
                this->_grid_fs->storeFile(filename, id.str());

                // Clean up the temporary file
                unlink(filename);
                free(filename);
            }

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
