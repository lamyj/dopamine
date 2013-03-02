/*
 *
 *  Copyright (C) 1993-2010, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module:  dcmqrdb
 *
 *  Author:  Marco Eichelberg
 *
 *  Purpose: class DcmQueryRetrieveSCP
 *
 */

#include <fstream>

// MongoDb is still using boost::filesystem v2
#define BOOST_FILESYSTEM_VERSION 2

#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>

#include <dcmtk/config/osconfig.h>    /* make sure OS specific configuration is included first */
#include <dcmtk/dcmqrdb/dcmqropt.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcmetinf.h>
#include <dcmtk/dcmnet/diutil.h>

#include "DataSetToBSON.h"
#include "DcmQueryRetrieveSCP.h"
#include "FindResponseGenerator.h"
#include "IsPrivateTag.h"
#include "VRMatch.h"

struct FindCallbackData
{
    DcmQueryRetrieveSCP * scp;
    std::string ae_title;
};

struct StoreCallbackData
{
    DIC_US status;
    DcmQueryRetrieveSCP * scp;
    std::string source_application_entity_title;
};

template<typename TIterator>
uint32_t hashCode(TIterator begin, TIterator end)
{
    uint32_t hash=0;
    TIterator it(begin);
    while(it != end)
    {   
        hash = 31*hash+(*it);
        ++it;
    }   
    return hash;
}

template<typename TString>
uint32_t hashCode(TString const & s)
{
    char const * const begin = s.c_str();
    char const * const end = begin+s.size();
    return hashCode(begin, end);
}

std::string hashToString(uint32_t hash)
{
    std::string hash_hex(9, ' '); // Use one more char for '\0'
    snprintf(&hash_hex[0], hash_hex.size(), "%08X", hash);
    return hash_hex;
}

static void findCallback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_FindRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_FindRSP *response,
        DcmDataset **responseIdentifiers,
        DcmDataset **stDetail)
{
    static FindResponseGenerator * generator = NULL;

    OFCondition dbcond = EC_Normal;

    DcmQueryRetrieveSCP * scp = reinterpret_cast<FindCallbackData*>(callbackData)->scp;
    std::string const & ae_title = reinterpret_cast<FindCallbackData*>(callbackData)->ae_title;

    if (responseCount == 1)
    {
        /* start the database search */
        DCMQRDB_INFO("Find SCP Request Identifiers:" << OFendl << DcmObject::PrintHelper(*requestIdentifiers));
        if(generator != NULL)
        {
            delete generator;
        }
        generator = new FindResponseGenerator(
            *requestIdentifiers, scp->get_connection(), scp->get_db_name());
    }

    /* only cancel if we have pending responses */
    if (cancelled && DICOM_PENDING_STATUS(generator->status()))
    {
        generator->cancel();
    }

    if (DICOM_PENDING_STATUS(generator->status())) {
        dbcond = generator->next(responseIdentifiers);
        if (dbcond.bad())
        {
             DCMQRDB_ERROR("findSCP: Database: nextFindResponse Failed ("
                     << DU_cfindStatusString(generator->status()) << "):");
        }
    }

    if (*responseIdentifiers != NULL)
    {

        if (! DU_putStringDOElement(*responseIdentifiers, DCM_RetrieveAETitle, ae_title.c_str()))
        {
            DCMQRDB_ERROR("DO: adding Retrieve AE Title");
        }
    }

    /* set response status */
    response->DimseStatus = generator->status();
    *stDetail = NULL; // TODO

    OFString str;
    DCMQRDB_INFO("Find SCP Response " << responseCount << " [status: "
            << DU_cfindStatusString(generator->status()) << "]");
    DCMQRDB_DEBUG(DIMSE_dumpMessage(str, *response, DIMSE_OUTGOING));
    if (DICOM_PENDING_STATUS(generator->status()) && (*responseIdentifiers != NULL))
    {
        DCMQRDB_DEBUG("Find SCP Response Identifiers:" << OFendl << DcmObject::PrintHelper(**responseIdentifiers));
    }
    if (*stDetail)
    {
        DCMQRDB_DEBUG("  Status detail:" << OFendl << DcmObject::PrintHelper(**stDetail));
    }
}


static void getCallback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_GetRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_GetRSP *response, DcmDataset **stDetail,
        DcmDataset **responseIdentifiers)
{
//    mongo::GridFile file = scp->get_grid_fs().findFile(id.str());
//    std::cout << file.exists() << std::endl;
//    std::cout << file.getContentLength() << std::endl;
//    file.write("foo.dcm");
    /*
  DcmQueryRetrieveGetContext *context = OFstatic_cast(DcmQueryRetrieveGetContext *, callbackData);
  context->callbackHandler(cancelled, request, requestIdentifiers, responseCount, response, stDetail, responseIdentifiers);
    */
}


static void moveCallback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_MoveRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_MoveRSP *response, DcmDataset **stDetail,
        DcmDataset **responseIdentifiers)
{
    /*
  DcmQueryRetrieveMoveContext *context = OFstatic_cast(DcmQueryRetrieveMoveContext *, callbackData);
  context->callbackHandler(cancelled, request, requestIdentifiers, responseCount, response, stDetail, responseIdentifiers);
    */
}


static void storeCallback(
    /* in */
    void *callbackData,
    T_DIMSE_StoreProgress *progress,    /* progress state */
    T_DIMSE_C_StoreRQ *req,             /* original store request */
    char *imageFileName,                /* being received into */
    DcmDataset **imageDataSet,          /* being received into */
    /* out */
    T_DIMSE_C_StoreRSP *rsp,            /* final store response */
    DcmDataset **stDetail)
{
    if(progress->state == DIMSE_StoreEnd)
    {
        DcmQueryRetrieveSCP * scp = reinterpret_cast<StoreCallbackData*>(callbackData)->scp;

        // Check if we already have this dataset, based on its SOP Instance UID
        OFString sop_instance_uid;
        (*imageDataSet)->findAndGetOFString(DcmTagKey(0x0008,0x0018), sop_instance_uid);

        mongo::auto_ptr<mongo::DBClientCursor> cursor =
            scp->get_connection().query(
                scp->get_db_name()+"."+"datasets",
                QUERY("00080018" << sop_instance_uid.c_str()));

        if(cursor->more())
        {
            // We already have this SOP Instance UID, do not store it
        }
        else
        {
            // Convert the dcmtk dataset to BSON
            DataSetToBSON converter;

            converter.get_filters().push_back(std::make_pair(
                IsPrivateTag::New(), DataSetToBSON::FilterAction::EXCLUDE));
            converter.get_filters().push_back(std::make_pair(
                VRMatch::New(EVR_OB), DataSetToBSON::FilterAction::EXCLUDE));
            converter.get_filters().push_back(std::make_pair(
                VRMatch::New(EVR_OF), DataSetToBSON::FilterAction::EXCLUDE));
            converter.get_filters().push_back(std::make_pair(
                VRMatch::New(EVR_OW), DataSetToBSON::FilterAction::EXCLUDE));
            converter.get_filters().push_back(std::make_pair(
                VRMatch::New(EVR_UN), DataSetToBSON::FilterAction::EXCLUDE));
            converter.set_default_filter(DataSetToBSON::FilterAction::INCLUDE);

            mongo::BSONObjBuilder builder;
            converter(*imageDataSet, builder);

            // Store it in the Mongo DB instance
            mongo::OID const id(mongo::OID::gen());
            builder << "_id" << id;

            // Create the header of the new file
            DcmFileFormat file_format(*imageDataSet);
            file_format.getMetaInfo()->putAndInsertString(DCM_SourceApplicationEntityTitle, 
               reinterpret_cast<StoreCallbackData*>(callbackData)->source_application_entity_title.c_str());

            // Compute the destination filename
            // DCM4CHEE does:
            // /root
            //   /year/month/day (of now)
            //   /(32-bits hash of Study Instance UID
            //   /(32-bits hash of Series Instance UID)
            //   /(32-bits hash of SOP Instance UID)
            // The 32-bits hash is based on String.hashCode (http://docs.oracle.com/javase/1.5.0/docs/api/java/lang/String.html#hashCode%28%29)

            boost::gregorian::date const today(
                boost::gregorian::day_clock::universal_day());

            OFString study_instance_uid;
            (*imageDataSet)->findAndGetOFStringArray(DCM_StudyInstanceUID, study_instance_uid);
            OFString series_instance_uid;
            (*imageDataSet)->findAndGetOFStringArray(DCM_SeriesInstanceUID, series_instance_uid);
            OFString sop_instance_uid;
            (*imageDataSet)->findAndGetOFStringArray(DCM_SOPInstanceUID, sop_instance_uid);

            std::string const study_hash = hashToString(hashCode(study_instance_uid));
            std::string const series_hash = hashToString(hashCode(series_instance_uid));
            std::string const sop_instance_hash = hashToString(hashCode(sop_instance_uid));

            boost::filesystem::path const destination =
                scp->get_storage()
                    /boost::lexical_cast<std::string>(today.year())
                    /boost::lexical_cast<std::string>(today.month().as_number())
                    /boost::lexical_cast<std::string>(today.day())
                    /study_hash/series_hash/sop_instance_hash;

            // Store the dataset in DB and in filesystem
            builder << "location" << destination.string();
            scp->get_connection().insert(scp->get_db_name()+".datasets", builder.obj());
            boost::filesystem::create_directories(destination.parent_path());
            file_format.saveFile(destination.string().c_str(), EXS_LittleEndianImplicit);
        }
    }
}


/*
 * ============================================================================================================
 */


DcmQueryRetrieveSCP::DcmQueryRetrieveSCP(
  const DcmQueryRetrieveConfig& config,
  const DcmQueryRetrieveOptions& options,
  DbConnection const & db_connection, boost::filesystem::path const & storage)
: config_(&config)
, dbCheckFindIdentifier_(OFFalse)
, dbCheckMoveIdentifier_(OFFalse)
, options_(options),
  _db_name(db_connection.db_name), _storage(storage)
{
    std::stringstream stream;
    stream << db_connection.port;
    this->_connection.connect(db_connection.host+":"+stream.str());

    std::string const datasets=this->_db_name+".datasets";
    this->_connection.ensureIndex(
        datasets, BSON("\"00080018\"" << 1), false, "SOP Instance UID");
    this->_connection.ensureIndex(
        datasets, BSON("\"00100010\"" << 1), false, "Patient's Name");
    this->_connection.ensureIndex(
        datasets, BSON("\"00100020\"" << 1), false, "Patient ID");

    this->_connection.ensureIndex(
        datasets, BSON("\"0020000e\"" << 1), false, "Series Instance UID");
    this->_connection.ensureIndex(
        datasets, BSON("\"0008103e\"" << 1), false, "Series Description");

    this->_connection.ensureIndex(
        datasets, BSON("\"0020000d\"" << 1), false, "Study Instance UID");
    this->_connection.ensureIndex(
        datasets, BSON("\"00081030\"" << 1), false, "Study Description");
}


OFCondition DcmQueryRetrieveSCP::dispatch(T_ASC_Association *assoc, OFBool correctUIDPadding)
{
    OFCondition cond = EC_Normal;
    T_DIMSE_Message msg;
    T_ASC_PresentationContextID presID;
    OFBool firstLoop = OFTrue;

    // this while loop is executed exactly once unless the "keepDBHandleDuringAssociation_"
    // flag is not set, in which case the inner loop is executed only once and this loop
    // repeats for each incoming DIMSE command. In this case, the DB handle is created
    // and released for each DIMSE command.
    while (cond.good())
    {
        firstLoop = OFTrue;

        // this while loop is executed exactly once unless the "keepDBHandleDuringAssociation_"
        // flag is set, in which case the DB handle remains open until something goes wrong
        // or the remote peer closes the association
        while (cond.good() && (firstLoop || options_.keepDBHandleDuringAssociation_) )
        {
            firstLoop = OFFalse;
            cond = DIMSE_receiveCommand(assoc, DIMSE_BLOCKING, 0, &presID, &msg, NULL);

            /* did peer release, abort, or do we have a valid message ? */
            if (cond.good())
            {
                /* process command */
                switch (msg.CommandField) {
                case DIMSE_C_ECHO_RQ:
                    cond = echoSCP(assoc, &msg.msg.CEchoRQ, presID);
                    break;
                case DIMSE_C_STORE_RQ:
                    cond = storeSCP(assoc, &msg.msg.CStoreRQ, presID, correctUIDPadding);
                    break;
                case DIMSE_C_FIND_RQ:
                    cond = findSCP(assoc, &msg.msg.CFindRQ, presID);
                    break;
//                case DIMSE_C_MOVE_RQ:
//                    cond = moveSCP(assoc, &msg.msg.CMoveRQ, presID, *dbHandle);
//                    break;
//                case DIMSE_C_GET_RQ:
//                    cond = getSCP(assoc, &msg.msg.CGetRQ, presID, *dbHandle);
//                    break;
                case DIMSE_C_CANCEL_RQ:
                    /* This is a late cancel request, just ignore it */
                    DCMQRDB_INFO("dispatch: late C-CANCEL-RQ, ignoring");
                    break;
                default:
                    /* we cannot handle this kind of message */
                    cond = DIMSE_BADCOMMANDTYPE;
                    DCMQRDB_ERROR("Cannot handle command: 0x" << STD_NAMESPACE hex <<
                            (unsigned)msg.CommandField);
                    /* the condition will be returned, the caller will abort the association. */
                }
            }
            else if ((cond == DUL_PEERREQUESTEDRELEASE)||(cond == DUL_PEERABORTEDASSOCIATION))
            {
                // association gone
            }
            else
            {
                // the condition will be returned, the caller will abort the assosiation.
            }
        }
    }

    // Association done
    return cond;
}


OFCondition DcmQueryRetrieveSCP::handleAssociation(T_ASC_Association * assoc, OFBool correctUIDPadding)
{
    OFCondition         cond = EC_Normal;
    DIC_NODENAME        peerHostName;
    DIC_AE              peerAETitle;
    DIC_AE              myAETitle;
    OFString            temp_str;

    ASC_getPresentationAddresses(assoc->params, peerHostName, NULL);
    ASC_getAPTitles(assoc->params, peerAETitle, myAETitle, NULL);

    /* now do the real work */
    cond = dispatch(assoc, correctUIDPadding);

    /* clean up on association termination */
    if (cond == DUL_PEERREQUESTEDRELEASE) {
        DCMQRDB_INFO("Association Release");
        cond = ASC_acknowledgeRelease(assoc);
        ASC_dropSCPAssociation(assoc);
    } else if (cond == DUL_PEERABORTEDASSOCIATION) {
        DCMQRDB_INFO("Association Aborted");
    } else {
        DCMQRDB_ERROR("DIMSE Failure (aborting association): " << DimseCondition::dump(temp_str, cond));
        /* some kind of error so abort the association */
        cond = ASC_abortAssociation(assoc);
    }

    cond = ASC_dropAssociation(assoc);
    if (cond.bad()) {
        DCMQRDB_ERROR("Cannot Drop Association: " << DimseCondition::dump(temp_str, cond));
    }
    cond = ASC_destroyAssociation(&assoc);
    if (cond.bad()) {
        DCMQRDB_ERROR("Cannot Destroy Association: " << DimseCondition::dump(temp_str, cond));
    }

    return cond;
}


OFCondition DcmQueryRetrieveSCP::echoSCP(T_ASC_Association * assoc, T_DIMSE_C_EchoRQ * req,
        T_ASC_PresentationContextID presId)
{
    OFCondition cond = EC_Normal;

    DCMQRDB_INFO("Received Echo SCP RQ: MsgID " << req->MessageID);
    /* we send an echo response back */
    cond = DIMSE_sendEchoResponse(assoc, presId,
        req, STATUS_Success, NULL);

    if (cond.bad()) {
        OFString temp_str;
        DCMQRDB_ERROR("echoSCP: Echo Response Failed: " << DimseCondition::dump(temp_str, cond));
    }
    return cond;
}


OFCondition DcmQueryRetrieveSCP::findSCP(T_ASC_Association * assoc, T_DIMSE_C_FindRQ * request,
        T_ASC_PresentationContextID presID)

{
    OFCondition cond = EC_Normal;

    FindCallbackData data;
    data.scp = this;

    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(assoc->params, NULL, aeTitle, NULL);
    data.ae_title = aeTitle;

    OFString temp_str;
    DCMQRDB_INFO("Received Find SCP:" << OFendl << DIMSE_dumpMessage(temp_str, *request, DIMSE_INCOMING));

    cond = DIMSE_findProvider(assoc, presID, request,
        findCallback, &data, options_.blockMode_, options_.dimse_timeout_);
    if (cond.bad()) {
        DCMQRDB_ERROR("Find SCP Failed: " << DimseCondition::dump(temp_str, cond));
    }
    return cond;
}


OFCondition DcmQueryRetrieveSCP::getSCP(T_ASC_Association * assoc, T_DIMSE_C_GetRQ * request,
        T_ASC_PresentationContextID presID, DcmQueryRetrieveDatabaseHandle& dbHandle)
{
    OFCondition cond = EC_Normal;
//    DcmQueryRetrieveGetContext context(dbHandle, options_, STATUS_Pending, assoc, request->MessageID, request->Priority, presID);

//    DIC_AE aeTitle;
//    aeTitle[0] = '\0';
//    ASC_getAPTitles(assoc->params, NULL, aeTitle, NULL);
//    context.setOurAETitle(aeTitle);

//    OFString temp_str;
//    DCMQRDB_INFO("Received Get SCP:" << OFendl << DIMSE_dumpMessage(temp_str, *request, DIMSE_INCOMING));

//    cond = DIMSE_getProvider(assoc, presID, request,
//        getCallback, &context, options_.blockMode_, options_.dimse_timeout_);
//    if (cond.bad()) {
//        DCMQRDB_ERROR("Get SCP Failed: " << DimseCondition::dump(temp_str, cond));
//    }
    return cond;
}


OFCondition DcmQueryRetrieveSCP::moveSCP(T_ASC_Association * assoc, T_DIMSE_C_MoveRQ * request,
        T_ASC_PresentationContextID presID, DcmQueryRetrieveDatabaseHandle& dbHandle)
{
    OFCondition cond = EC_Normal;
//    DcmQueryRetrieveMoveContext context(dbHandle, options_, config_, STATUS_Pending, assoc, request->MessageID, request->Priority);

//    DIC_AE aeTitle;
//    aeTitle[0] = '\0';
//    ASC_getAPTitles(assoc->params, NULL, aeTitle, NULL);
//    context.setOurAETitle(aeTitle);

//    OFString temp_str;
//    DCMQRDB_INFO("Received Move SCP:" << OFendl << DIMSE_dumpMessage(temp_str, *request, DIMSE_INCOMING));

//    cond = DIMSE_moveProvider(assoc, presID, request,
//        moveCallback, &context, options_.blockMode_, options_.dimse_timeout_);
//    if (cond.bad()) {
//        DCMQRDB_ERROR("Move SCP Failed: " << DimseCondition::dump(temp_str, cond));
//    }
    return cond;
}


OFCondition DcmQueryRetrieveSCP::storeSCP(T_ASC_Association * assoc, T_DIMSE_C_StoreRQ * request,
             T_ASC_PresentationContextID presId,
             OFBool correctUIDPadding)
{
    OFCondition cond = EC_Normal;

    OFString temp_str;
    DCMQRDB_INFO("Received Store SCP:" << OFendl << DIMSE_dumpMessage(temp_str, *request, DIMSE_INCOMING));

    StoreCallbackData data;
    data.scp = this;

    if (!dcmIsaStorageSOPClassUID(request->AffectedSOPClassUID))
    {
        /* callback will send back sop class not supported status */
        data.status = STATUS_STORE_Refused_SOPClassNotSupported;
    }

    // store SourceApplicationEntityTitle in metaheader
    if (assoc && assoc->params)
    {
        char const * aet = assoc->params->DULparams.callingAPTitle;
        if(aet != NULL)
        {
            data.source_application_entity_title = assoc->params->DULparams.callingAPTitle;
        }
    }

    /* we must still retrieve the data set even if some error has occured */
    DcmDataset dset;
    DcmDataset * dset_ptr = &dset;
    cond = DIMSE_storeProvider(assoc, presId, request, (char *)NULL,
        (int)options_.useMetaheader_, &dset_ptr, storeCallback, (void*)&data,
        options_.blockMode_, options_.dimse_timeout_);

    if (cond.bad())
    {
        DCMQRDB_ERROR("Store SCP Failed: " << DimseCondition::dump(temp_str, cond));
    }

    return cond;
}


/* Association negotiation */

void DcmQueryRetrieveSCP::refuseAnyStorageContexts(T_ASC_Association * assoc)
{
    int i;
    T_ASC_PresentationContextID pid;

    for (i = 0; i < numberOfAllDcmStorageSOPClassUIDs; i++) {
        do {
          pid = ASC_findAcceptedPresentationContextID(assoc, dcmAllStorageSOPClassUIDs[i]);
          if (pid != 0) ASC_refusePresentationContext(assoc->params, pid, ASC_P_USERREJECTION);
        } while (pid != 0); // repeat as long as we find presentation contexts for this SOP class - there might be multiple ones.
    }
}


OFCondition DcmQueryRetrieveSCP::refuseAssociation(T_ASC_Association ** assoc, CTN_RefuseReason reason)
{
    OFCondition cond = EC_Normal;
    T_ASC_RejectParameters rej;
    OFString temp_str;

    const char *reason_string;
    switch (reason)
    {
      case CTN_TooManyAssociations:
          reason_string = "TooManyAssociations";
          break;
      case CTN_CannotFork:
          reason_string = "CannotFork";
          break;
      case CTN_BadAppContext:
          reason_string = "BadAppContext";
          break;
      case CTN_BadAEPeer:
          reason_string = "BadAEPeer";
          break;
      case CTN_BadAEService:
          reason_string = "BadAEService";
          break;
      case CTN_NoReason:
          reason_string = "NoReason";
        break;
      default:
          reason_string = "???";
          break;
    }
    DCMQRDB_INFO("Refusing Association (" << reason_string << ")");

    switch (reason)
    {
      case CTN_TooManyAssociations:
        rej.result = ASC_RESULT_REJECTEDTRANSIENT;
        rej.source = ASC_SOURCE_SERVICEPROVIDER_PRESENTATION_RELATED;
        rej.reason = ASC_REASON_SP_PRES_LOCALLIMITEXCEEDED;
        break;
      case CTN_CannotFork:
        rej.result = ASC_RESULT_REJECTEDPERMANENT;
        rej.source = ASC_SOURCE_SERVICEPROVIDER_PRESENTATION_RELATED;
        rej.reason = ASC_REASON_SP_PRES_TEMPORARYCONGESTION;
        break;
      case CTN_BadAppContext:
        rej.result = ASC_RESULT_REJECTEDTRANSIENT;
        rej.source = ASC_SOURCE_SERVICEUSER;
        rej.reason = ASC_REASON_SU_APPCONTEXTNAMENOTSUPPORTED;
        break;
      case CTN_BadAEPeer:
        rej.result = ASC_RESULT_REJECTEDPERMANENT;
        rej.source = ASC_SOURCE_SERVICEUSER;
        rej.reason = ASC_REASON_SU_CALLINGAETITLENOTRECOGNIZED;
        break;
      case CTN_BadAEService:
        rej.result = ASC_RESULT_REJECTEDPERMANENT;
        rej.source = ASC_SOURCE_SERVICEUSER;
        rej.reason = ASC_REASON_SU_CALLEDAETITLENOTRECOGNIZED;
        break;
      case CTN_NoReason:
      default:
        rej.result = ASC_RESULT_REJECTEDPERMANENT;
        rej.source = ASC_SOURCE_SERVICEUSER;
        rej.reason = ASC_REASON_SU_NOREASON;
        break;
    }

    cond = ASC_rejectAssociation(*assoc, &rej);

    if (cond.bad())
    {
      DCMQRDB_ERROR("Association Reject Failed: " << DimseCondition::dump(temp_str, cond));
    }

    cond = ASC_dropAssociation(*assoc);
    if (cond.bad())
    {
      DCMQRDB_ERROR("Cannot Drop Association: " << DimseCondition::dump(temp_str, cond));
    }
    cond = ASC_destroyAssociation(assoc);
    if (cond.bad())
    {
      DCMQRDB_ERROR("Cannot Destroy Association: " << DimseCondition::dump(temp_str, cond));
    }

    return cond;
}


OFCondition DcmQueryRetrieveSCP::negotiateAssociation(T_ASC_Association * assoc)
{
    OFCondition cond = EC_Normal;
    int i;
    T_ASC_PresentationContextID movepid, findpid;
    OFString temp_str;
    struct { const char *moveSyntax, *findSyntax; } queryRetrievePairs[] =
    {
      { UID_MOVEPatientRootQueryRetrieveInformationModel,
        UID_FINDPatientRootQueryRetrieveInformationModel },
      { UID_MOVEStudyRootQueryRetrieveInformationModel,
        UID_FINDStudyRootQueryRetrieveInformationModel },
      { UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel,
        UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel }
    };

    DIC_AE calledAETitle;
    ASC_getAPTitles(assoc->params, NULL, calledAETitle, NULL);

    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };
    int numTransferSyntaxes = 0;

    switch (options_.networkTransferSyntax_)
    {
      case EXS_LittleEndianImplicit:
        /* we only support Little Endian Implicit */
        transferSyntaxes[0]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 1;
        break;
      case EXS_LittleEndianExplicit:
        /* we prefer Little Endian Explicit */
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[2]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
      case EXS_BigEndianExplicit:
        /* we prefer Big Endian Explicit */
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2]  = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
#ifndef DISABLE_COMPRESSION_EXTENSION
      case EXS_JPEGProcess14SV1TransferSyntax:
        /* we prefer JPEGLossless:Hierarchical-1stOrderPrediction (default lossless) */
        transferSyntaxes[0] = UID_JPEGProcess14SV1TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEGProcess1TransferSyntax:
        /* we prefer JPEGBaseline (default lossy for 8 bit images) */
        transferSyntaxes[0] = UID_JPEGProcess1TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEGProcess2_4TransferSyntax:
        /* we prefer JPEGExtended (default lossy for 12 bit images) */
        transferSyntaxes[0] = UID_JPEGProcess2_4TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEG2000LosslessOnly:
        /* we prefer JPEG 2000 lossless */
        transferSyntaxes[0] = UID_JPEG2000LosslessOnlyTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEG2000:
        /* we prefer JPEG 2000 lossy or lossless */
        transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEGLSLossless:
        /* we prefer JPEG-LS Lossless */
        transferSyntaxes[0] = UID_JPEGLSLosslessTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_JPEGLSLossy:
        /* we prefer JPEG-LS Lossy */
        transferSyntaxes[0] = UID_JPEGLSLossyTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_MPEG2MainProfileAtMainLevel:
        /* we prefer MPEG2 MP@ML */
        transferSyntaxes[0] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_MPEG2MainProfileAtHighLevel:
        /* we prefer MPEG2 MP@HL */
        transferSyntaxes[0] = UID_MPEG2MainProfileAtHighLevelTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
      case EXS_RLELossless:
        /* we prefer RLE Lossless */
        transferSyntaxes[0] = UID_RLELosslessTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
#ifdef WITH_ZLIB
      case EXS_DeflatedLittleEndianExplicit:
        /* we prefer deflated transmission */
        transferSyntaxes[0] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
#endif
#endif
      default:
        /* We prefer explicit transfer syntaxes.
         * If we are running on a Little Endian machine we prefer
         * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
         */
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
        break;
    }

    const char * const nonStorageSyntaxes[] =
    {
        UID_VerificationSOPClass,
        UID_FINDPatientRootQueryRetrieveInformationModel,
        UID_MOVEPatientRootQueryRetrieveInformationModel,
        UID_GETPatientRootQueryRetrieveInformationModel,
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
        UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel,
        UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel,
        UID_RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel,
#endif
        UID_FINDStudyRootQueryRetrieveInformationModel,
        UID_MOVEStudyRootQueryRetrieveInformationModel,
        UID_GETStudyRootQueryRetrieveInformationModel,
        UID_PrivateShutdownSOPClass
    };

    const int numberOfNonStorageSyntaxes = DIM_OF(nonStorageSyntaxes);
    const char *selectedNonStorageSyntaxes[DIM_OF(nonStorageSyntaxes)];
    int numberOfSelectedNonStorageSyntaxes = 0;
    for (i = 0; i < numberOfNonStorageSyntaxes; i++)
    {
        if (0 == strcmp(nonStorageSyntaxes[i], UID_FINDPatientRootQueryRetrieveInformationModel))
        {
          if (options_.supportPatientRoot_) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_MOVEPatientRootQueryRetrieveInformationModel))
        {
          if (options_.supportPatientRoot_) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_GETPatientRootQueryRetrieveInformationModel))
        {
          if (options_.supportPatientRoot_ && (! options_.disableGetSupport_)) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel))
        {
          if (options_.supportPatientStudyOnly_) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel))
        {
          if (options_.supportPatientStudyOnly_) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel))
        {
          if (options_.supportPatientStudyOnly_ && (! options_.disableGetSupport_)) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_FINDStudyRootQueryRetrieveInformationModel))
        {
          if (options_.supportStudyRoot_) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_MOVEStudyRootQueryRetrieveInformationModel))
        {
          if (options_.supportStudyRoot_) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_GETStudyRootQueryRetrieveInformationModel))
        {
          if (options_.supportStudyRoot_ && (! options_.disableGetSupport_)) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
        else if (0 == strcmp(nonStorageSyntaxes[i], UID_PrivateShutdownSOPClass))
        {
          if (options_.allowShutdown_) selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        } else {
            selectedNonStorageSyntaxes[numberOfSelectedNonStorageSyntaxes++] = nonStorageSyntaxes[i];
        }
    }

    /*  accept any of the non-storage syntaxes */
    cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
    assoc->params,
    (const char**)selectedNonStorageSyntaxes, numberOfSelectedNonStorageSyntaxes,
    (const char**)transferSyntaxes, numTransferSyntaxes);
    if (cond.bad()) {
        DCMQRDB_ERROR("Cannot accept presentation contexts: " << DimseCondition::dump(temp_str, cond));
    }

    /*  accept any of the storage syntaxes */
    if (options_.disableGetSupport_)
    {
      /* accept storage syntaxes with default role only */
      cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
        assoc->params,
        dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs,
        (const char**)transferSyntaxes, DIM_OF(transferSyntaxes));
      if (cond.bad()) {
        DCMQRDB_ERROR("Cannot accept presentation contexts: " << DimseCondition::dump(temp_str, cond));
      }
    } else {
      /* accept storage syntaxes with proposed role */
      T_ASC_PresentationContext pc;
      T_ASC_SC_ROLE role;
      int npc = ASC_countPresentationContexts(assoc->params);
      for (i = 0; i < npc; i++)
      {
        ASC_getPresentationContext(assoc->params, i, &pc);
        if (dcmIsaStorageSOPClassUID(pc.abstractSyntax))
        {
          /*
          ** We are prepared to accept whatever role he proposes.
          ** Normally we can be the SCP of the Storage Service Class.
          ** When processing the C-GET operation we can be the SCU of the Storage Service Class.
          */
          role = pc.proposedRole;

          /*
          ** Accept in the order "least wanted" to "most wanted" transfer
          ** syntax.  Accepting a transfer syntax will override previously
          ** accepted transfer syntaxes.
          */
          for (int k = numTransferSyntaxes - 1; k >= 0; k--)
          {
            for (int j = 0; j < (int)pc.transferSyntaxCount; j++)
            {
              /* if the transfer syntax was proposed then we can accept it
               * appears in our supported list of transfer syntaxes
               */
              if (strcmp(pc.proposedTransferSyntaxes[j], transferSyntaxes[k]) == 0)
              {
                cond = ASC_acceptPresentationContext(
                    assoc->params, pc.presentationContextID, transferSyntaxes[k], role);
                if (cond.bad()) return cond;
              }
            }
          }
        }
      } /* for */
    } /* else */

    /*
     * check if we have negotiated the private "shutdown" SOP Class
     */
    if (0 != ASC_findAcceptedPresentationContextID(assoc, UID_PrivateShutdownSOPClass))
    {
      DCMQRDB_INFO("Shutting down server ... (negotiated private \"shut down\" SOP class)");
      refuseAssociation(&assoc, CTN_NoReason);
      return ASC_SHUTDOWNAPPLICATION;
    }

    /*
     * Refuse any "Storage" presentation contexts to non-writable
     * storage areas.
     */
    if (!config_->writableStorageArea(calledAETitle))
    {
      refuseAnyStorageContexts(assoc);
    }

    /*
     * Enforce RSNA'93 Demonstration Requirements about only
     * accepting a context for MOVE if a context for FIND is also present.
     */

    for (i = 0; i < (int)DIM_OF(queryRetrievePairs); i++) {
        movepid = ASC_findAcceptedPresentationContextID(assoc,
        queryRetrievePairs[i].moveSyntax);
        if (movepid != 0) {
          findpid = ASC_findAcceptedPresentationContextID(assoc,
              queryRetrievePairs[i].findSyntax);
          if (findpid == 0) {
            if (options_.requireFindForMove_) {
              /* refuse the move */
              ASC_refusePresentationContext(assoc->params,
                  movepid, ASC_P_USERREJECTION);
            } else {
              DCMQRDB_ERROR("Move Presentation Context but no Find (accepting for now)");
            }
          }
        }
    }

    /*
     * Enforce an Ad-Hoc rule to limit storage access.
     * If the storage area is "writable" and some other association has
     * already negotiated a "Storage" class presentation context,
     * then refuse any "storage" presentation contexts.
     */

    if (options_.refuseMultipleStorageAssociations_)
    {
        if (config_->writableStorageArea(calledAETitle))
        {
          if (processtable_.haveProcessWithWriteAccess(calledAETitle))
          {
            refuseAnyStorageContexts(assoc);
          }
        }
    }

    return cond;
}


OFCondition DcmQueryRetrieveSCP::waitForAssociation(T_ASC_Network * theNet)
{
    OFCondition cond = EC_Normal;
    OFString temp_str;
#ifdef HAVE_FORK
    int                 pid;
#endif
    T_ASC_Association  *assoc;
    char                buf[BUFSIZ];
    int timeout;
    OFBool go_cleanup = OFFalse;

    if (options_.singleProcess_) timeout = 1000;
    else
    {
      if (processtable_.countChildProcesses() > 0)
      {
        timeout = 1;
      } else {
        timeout = 1000;
      }
    }

    if (ASC_associationWaiting(theNet, timeout))
    {
        cond = ASC_receiveAssociation(theNet, &assoc, (int)options_.maxPDU_);
        if (cond.bad())
        {
          DCMQRDB_INFO("Failed to receive association: " << DimseCondition::dump(temp_str, cond));
          go_cleanup = OFTrue;
        }
    } else return EC_Normal;

    if (! go_cleanup)
    {
        time_t t = time(NULL);
        DCMQRDB_INFO("Association Received (" << assoc->params->DULparams.callingPresentationAddress
                << ":" << assoc->params->DULparams.callingAPTitle << " -> "
                << assoc->params->DULparams.calledAPTitle << ") " << ctime(&t));

        DCMQRDB_DEBUG("Parameters:" << OFendl << ASC_dumpParameters(temp_str, assoc->params, ASC_ASSOC_RQ));

        if (options_.refuse_)
        {
            DCMQRDB_INFO("Refusing Association (forced via command line)");
            cond = refuseAssociation(&assoc, CTN_NoReason);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        /* Application Context Name */
        cond = ASC_getApplicationContextName(assoc->params, buf);
        if (cond.bad() || strcmp(buf, DICOM_STDAPPLICATIONCONTEXT) != 0)
        {
            /* reject: the application context name is not supported */
            DCMQRDB_INFO("Bad AppContextName: " << buf);
            cond = refuseAssociation(&assoc, CTN_BadAppContext);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        /* Implementation Class UID */
        if (options_.rejectWhenNoImplementationClassUID_ &&
        strlen(assoc->params->theirImplementationClassUID) == 0)
        {
            /* reject: no implementation Class UID provided */
            DCMQRDB_INFO("No implementation Class UID provided");
            cond = refuseAssociation(&assoc, CTN_NoReason);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        /* Does peer AE have access to required service ?? */
        if (! config_->peerInAETitle(assoc->params->DULparams.calledAPTitle,
        assoc->params->DULparams.callingAPTitle,
        assoc->params->DULparams.callingPresentationAddress))
        {
            cond = refuseAssociation(&assoc, CTN_BadAEService);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        // too many concurrent associations ??
        if (processtable_.countChildProcesses() >= OFstatic_cast(size_t, options_.maxAssociations_))
        {
            cond = refuseAssociation(&assoc, CTN_TooManyAssociations);
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        cond = negotiateAssociation(assoc);
        if (cond.bad()) go_cleanup = OFTrue;
    }

    if (! go_cleanup)
    {
        cond = ASC_acknowledgeAssociation(assoc);
        if (cond.bad())
        {
            DCMQRDB_ERROR(DimseCondition::dump(temp_str, cond));
            go_cleanup = OFTrue;
        }
    }

    if (! go_cleanup)
    {
        DCMQRDB_INFO("Association Acknowledged (Max Send PDV: " << assoc->sendPDVLength << ")");
        if (ASC_countAcceptedPresentationContexts(assoc->params) == 0)
            DCMQRDB_INFO("    (but no valid presentation contexts)");
        DCMQRDB_DEBUG(ASC_dumpParameters(temp_str, assoc->params, ASC_ASSOC_AC));

        if (options_.singleProcess_)
        {
            /* don't spawn a sub-process to handle the association */
            cond = handleAssociation(assoc, options_.correctUIDPadding_);
        }
#ifdef HAVE_FORK
        else
        {
            /* spawn a sub-process to handle the association */
            pid = (int)(fork());
            if (pid < 0)
            {
                char errBuf[256];
                DCMQRDB_ERROR("Cannot create association sub-process: "
                   << OFStandard::strerror(errno, errBuf, sizeof(errBuf)));
                cond = refuseAssociation(&assoc, CTN_CannotFork);
                go_cleanup = OFTrue;
            }
            else if (pid > 0)
            {
                /* parent process, note process in table */
                processtable_.addProcessToTable(pid, assoc);
            }
            else
            {
                // Prepare child process to generate mongodb OIDs
                mongo::OID::justForked();
                /* child process, handle the association */
                cond = handleAssociation(assoc, options_.correctUIDPadding_);
                /* the child process is done so exit */
                exit(0);
            }
        }
#endif
    }

    // cleanup code
    OFCondition oldcond = cond;    /* store condition flag for later use */
    if (!options_.singleProcess_ && (cond != ASC_SHUTDOWNAPPLICATION))
    {
        /* the child will handle the association, we can drop it */
        cond = ASC_dropAssociation(assoc);
        if (cond.bad())
        {
            DCMQRDB_ERROR("Cannot Drop Association: " << DimseCondition::dump(temp_str, cond));
        }
        cond = ASC_destroyAssociation(&assoc);
        if (cond.bad())
        {
            DCMQRDB_ERROR("Cannot Destroy Association: " << DimseCondition::dump(temp_str, cond));
        }
    }

    if (oldcond == ASC_SHUTDOWNAPPLICATION) cond = oldcond; /* abort flag is reported to top-level wait loop */
    return cond;
}


void DcmQueryRetrieveSCP::cleanChildren()
{
  processtable_.cleanChildren();
}


void DcmQueryRetrieveSCP::setDatabaseFlags(
  OFBool dbCheckFindIdentifier,
  OFBool dbCheckMoveIdentifier)
{
  dbCheckFindIdentifier_ = dbCheckFindIdentifier;
  dbCheckMoveIdentifier_ = dbCheckMoveIdentifier;
}

mongo::DBClientConnection const &
DcmQueryRetrieveSCP
::get_connection() const
{
    return this->_connection;
}

mongo::DBClientConnection &
DcmQueryRetrieveSCP
::get_connection()
{
    return this->_connection;
}

std::string const &
DcmQueryRetrieveSCP
::get_db_name() const
{
    return this->_db_name;
}

boost::filesystem::path const &
DcmQueryRetrieveSCP
::get_storage() const
{
    return this->_storage;
}

DcmQueryRetrieveOptions const &
DcmQueryRetrieveSCP
::get_options() const
{
    return this->options_;
}
