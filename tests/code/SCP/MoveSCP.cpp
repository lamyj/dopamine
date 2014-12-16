/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#define BOOST_TEST_MODULE ModuleMoveSCP
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "SCP/MoveSCP.h"
#include "ToolsForTests.h"

/**
 * Pre-conditions:
 *     - we assume that ConfigurationPACS works correctly
 *     - we assume that AuthenticatorNone works correctly
 *     - we assume that DBConnection works correctly
 */

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    research_pacs::MoveSCP * movescp =
            new research_pacs::MoveSCP(NULL, T_ASC_PresentationContextID(), NULL);

    BOOST_REQUIRE_EQUAL(movescp != NULL, true);

    delete movescp;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Execute Move
 */
struct TestDataOK02
{
    DcmDataset * dataset;

    TestDataOK02()
    {
        // Create Configuration file
        std::ofstream myfile;
        myfile.open(NetworkConfFILE);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "allowed_peers=*\n";
        myfile << "port=11112\n";
        myfile << "[database]\n";
        myfile << "hostname=localhost\n";
        myfile << "port=27017\n";
        myfile << "dbname=pacs\n";
        myfile << "indexlist=SOPInstanceUID\n";
        myfile << "[authenticator]\n";
        myfile << "type=None\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=REMOTE,LOCAL\n";
        myfile << "REMOTE=localhost:11112\n";
        myfile << "LOCAL=localhost:11113\n";
        myfile.close();

        // Start NetworkPACS (create and launch thread)
        boost::thread networkThread(launchNetwork);
        sleep(1); // Wait network initialisation

        // Create Dataset To Move
        dataset = new DcmDataset();
        dataset->putAndInsertOFStringArray(DCM_PatientName, "Auto*");
        dataset->putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "PATIENT");
    }

    ~TestDataOK02()
    {
        delete dataset;

        remove(NetworkConfFILE.c_str());

        sleep(1);
        boost::filesystem::remove_all("./temp_dir");

        terminateNetwork();
    }
};

struct StoreCallbackData
{
  char* imageFileName;
  DcmFileFormat* dcmff;
  T_ASC_Association* assoc;
};

void
storeSCPCallback(
    /* in */
    void *callbackData,
    T_DIMSE_StoreProgress *progress,    /* progress state */
    T_DIMSE_C_StoreRQ *req,             /* original store request */
    char *imageFileName, DcmDataset **imageDataSet, /* being received into */
    /* out */
    T_DIMSE_C_StoreRSP *rsp,            /* final store response */
    DcmDataset **statusDetail)
{
    if (progress->state == DIMSE_StoreEnd)
    {
       *statusDetail = NULL;    /* no status detail */
    }
}

OFCondition storeSCP(
  T_ASC_Association *assoc,
  T_DIMSE_Message *msg,
  T_ASC_PresentationContextID presID)
{
    OFCondition cond = EC_Normal;
    T_DIMSE_C_StoreRQ *req;
    char imageFileName[2048];

    req = &msg->msg.CStoreRQ;

    sprintf(imageFileName, "%s.%s",
        dcmSOPClassUIDToModality(req->AffectedSOPClassUID),
        req->AffectedSOPInstanceUID);

    StoreCallbackData callbackData;
    callbackData.assoc = assoc;
    callbackData.imageFileName = imageFileName;
    DcmFileFormat dcmff;
    callbackData.dcmff = &dcmff;

    // store SourceApplicationEntityTitle in metaheader
    if (assoc && assoc->params)
    {
      const char *aet = assoc->params->DULparams.callingAPTitle;
      if (aet) dcmff.getMetaInfo()->putAndInsertString(DCM_SourceApplicationEntityTitle, aet);
    }

    DcmDataset *dset = dcmff.getDataset();

    cond = DIMSE_storeProvider(assoc, presID, req, NULL, 0,
                               &dset, storeSCPCallback, (void*)&callbackData,
                               DIMSE_BLOCKING, 30);

    if (cond.bad())
    {
        std::stringstream stream;
        stream << "Cannot store dataset: " << cond.text();
        throw research_pacs::ExceptionPACS(stream.str());
    }

    return cond;
}

void
subOpCallback(void * /*subOpCallbackData*/ ,
        T_ASC_Network *aNet, T_ASC_Association **subAssoc)
{
    if (aNet == NULL) return;   /* help no net ! */

    if (*subAssoc == NULL) {
        /* negotiate association */
        const char* knownAbstractSyntaxes[] = {
            UID_VerificationSOPClass
        };
        const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };
        int numTransferSyntaxes;

        OFCondition cond = ASC_receiveAssociation(aNet, subAssoc, ASC_DEFAULTMAXPDU);
        if (cond.good())
        {
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


            /* accept the Verification SOP Class if presented */
            cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
                (*subAssoc)->params,
                knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes),
                transferSyntaxes, numTransferSyntaxes);

            if (cond.good())
            {
                /* the array of Storage SOP Class UIDs comes from dcuid.h */
                cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
                    (*subAssoc)->params,
                    dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs,
                    transferSyntaxes, numTransferSyntaxes);
            }
        }
        if (cond.good()) cond = ASC_acknowledgeAssociation(*subAssoc);
        if (cond.bad()) {
            ASC_dropAssociation(*subAssoc);
            ASC_destroyAssociation(subAssoc);
        }
    }
    else
    {
        T_DIMSE_Message     msg;
        T_ASC_PresentationContextID presID;

        if (!ASC_dataWaiting(*subAssoc, 0)) /* just in case */
            return;

        OFCondition cond = DIMSE_receiveCommand(*subAssoc, DIMSE_BLOCKING, 30, &presID,
                &msg, NULL);

        if (cond == EC_Normal) {
          switch (msg.CommandField)
          {
            case DIMSE_C_STORE_RQ:
              cond = storeSCP(*subAssoc, &msg, presID);
              break;
            case DIMSE_C_ECHO_RQ:
              break;
            default:
              cond = DIMSE_BADCOMMANDTYPE;
              std::cout << "cannot handle command: 0x"
                   << STD_NAMESPACE hex << OFstatic_cast(unsigned, msg.CommandField) << std::endl;
              break;
          }
        }
        /* clean up on association termination */
        if (cond == DUL_PEERREQUESTEDRELEASE)
        {
            cond = ASC_acknowledgeRelease(*subAssoc);
            ASC_dropSCPAssociation(*subAssoc);
            ASC_destroyAssociation(subAssoc);
            return;
        }
        else if (cond == DUL_PEERABORTEDASSOCIATION)
        {
        }
        else if (cond != EC_Normal)
        {
            OFString temp_str;
            std::cout << "DIMSE failure (aborting sub-association): " << DimseCondition::dump(temp_str, cond) << std::endl;
            /* some kind of error so abort the association */
            cond = ASC_abortAssociation(*subAssoc);
        }

        if (cond != EC_Normal)
        {
            ASC_dropAssociation(*subAssoc);
            ASC_destroyAssociation(subAssoc);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK02)
{
    OFCondition condition;

    T_ASC_Network * networkSCU;
    condition = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, 11113, 30,
                                      &networkSCU);
    BOOST_CHECK_EQUAL(condition.good(), true);

    T_ASC_Parameters * params;
    condition = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_setAPTitles(params, "LOCAL", "REMOTE", NULL);
    BOOST_CHECK_EQUAL(condition.good(), true);

    std::string localhost(128, '\0');
    gethostname(&localhost[0], localhost.size()-1);

    condition = ASC_setPresentationAddresses(params, localhost.c_str(),
                                             "localhost:11112");

    typedef std::pair<std::string, std::vector<std::string> > PresentationContext;
    std::vector<PresentationContext> presentation_contexts;
    std::vector<std::string> tempvect = { UID_LittleEndianImplicitTransferSyntax,
                                          UID_LittleEndianExplicitTransferSyntax,
                                          UID_BigEndianExplicitTransferSyntax};
    presentation_contexts.push_back(std::make_pair(UID_MRImageStorage,
                                                   tempvect));

    unsigned int context_id = 1;
    for(auto const & context: presentation_contexts)
    {
        char const ** transfer_syntaxes = new char const *[context.second.size()];
        for(std::size_t i = 0; i < context.second.size(); ++i)
        {
            transfer_syntaxes[i] = context.second[i].c_str();
        }

        condition = ASC_addPresentationContext(params, context_id,
                                               context.first.c_str(),
                                               transfer_syntaxes,
                                               context.second.size());
        BOOST_CHECK_EQUAL(condition.good(), true);

        context_id += 2;
    }

    T_ASC_Association * association;
    condition = ASC_requestAssociation(networkSCU, params, &association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    T_ASC_PresentationContextID const presentation_id =
        ASC_findAcceptedPresentationContextID(association,
                                              UID_MRImageStorage);
    BOOST_CHECK_EQUAL(presentation_id != 0, true);

    T_ASC_PresentationContext pc;
    condition = ASC_findAcceptedPresentationContext(association->params,
                                                    presentation_id, &pc);
    BOOST_CHECK_EQUAL(presentation_id != 0, true);

    DIC_US const message_id = association->nextMsgID++;

    T_DIMSE_C_MoveRQ * request = new T_DIMSE_C_MoveRQ();
    memset(request, 0, sizeof(*request));
    request->MessageID = message_id;
    strcpy(request->AffectedSOPClassUID, UID_MRImageStorage);

    request->DataSetType = DIMSE_DATASET_PRESENT;
    request->Priority = DIMSE_PRIORITY_MEDIUM;

    condition = ASC_getAPTitles(association->params, request->MoveDestination,
                NULL, NULL);
    BOOST_CHECK_EQUAL(condition.good(), true);

    T_DIMSE_C_MoveRSP response;
    DcmDataset *detail = NULL;
    DcmDataset *rspIds = NULL;

    condition = DIMSE_moveUser(association, presentation_id, request, dataset,
                               NULL, NULL, DIMSE_BLOCKING, 30, networkSCU, subOpCallback,
                               NULL, &response, &detail, &rspIds, OFTrue);
    BOOST_CHECK_EQUAL(condition.good(), true);
    BOOST_CHECK_EQUAL(detail == NULL, true);
    BOOST_CHECK_EQUAL(response.DimseStatus, STATUS_Success);

    condition = ASC_releaseAssociation(association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_destroyAssociation(&association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_dropNetwork(&networkSCU);
    BOOST_CHECK_EQUAL(condition.good(), true);
}
