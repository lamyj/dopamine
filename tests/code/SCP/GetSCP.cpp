/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#define BOOST_TEST_MODULE ModuleGetSCP
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "SCP/GetSCP.h"
#include "ToolsForTests.h"

/**
 * Pre-conditions:
 *     - we assume that ConfigurationPACS works correctly
 *     - we assume that AuthenticatorNone works correctly
 *     - we assume that DBConnection works correctly
 *
 *     - Following Environment variables should be defined
 *          * DOPAMINE_TEST_LISTENINGPORT
 *          * DOPAMINE_TEST_WRITINGPORT
 *          * DOPAMINE_TEST_CONFIG
 */

/*************************** Tools functions **************************/
void
storeSCPCallback(
    /* in */
    void *callbackData,
    T_DIMSE_StoreProgress *progress,                /* progress state */
    T_DIMSE_C_StoreRQ *req,                         /* original store request */
    char *imageFileName, DcmDataset **imageDataSet, /* being received into */
    /* out */
    T_DIMSE_C_StoreRSP *rsp,                        /* final store response */
    DcmDataset **statusDetail)
{
    if (progress->state == DIMSE_StoreEnd)
    {
       *statusDetail = NULL;    /* no status detail */
    }
}

OFCondition
storeSCP(T_ASC_Association *assoc,
         T_DIMSE_Message *msg,
         T_ASC_PresentationContextID presID)
{
    T_DIMSE_C_StoreRQ *req = &msg->msg.CStoreRQ;

    DcmFileFormat dcmff;
    DcmDataset *dset = dcmff.getDataset();

    OFCondition cond = DIMSE_storeProvider(assoc, presID, req, NULL, 0,
                                           &dset, storeSCPCallback, NULL,
                                           DIMSE_BLOCKING, 30);
    if (cond.bad())
    {
        std::stringstream stream;
        stream << "Cannot store dataset: " << cond.text();
        throw dopamine::ExceptionPACS(stream.str());
    }

    return cond;
}

void
subOpCallback(void * /*subOpCallbackData*/ ,
              T_ASC_Network *aNet, T_ASC_Association **subAssoc)
{
    if (aNet == NULL) return;

    if (*subAssoc == NULL)
    {
        const char* knownAbstractSyntaxes[] =
        {
            UID_VerificationSOPClass
        };

        OFCondition cond = ASC_receiveAssociation(aNet, subAssoc, ASC_DEFAULTMAXPDU);
        if (cond.good())
        {
            const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };
            int numTransferSyntaxes;
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

            /* accept the Verification SOP Class if presented */
            cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
                        (*subAssoc)->params, knownAbstractSyntaxes,
                        DIM_OF(knownAbstractSyntaxes),
                        transferSyntaxes, numTransferSyntaxes);

            if (cond.good())
            {
                /* the array of Storage SOP Class UIDs comes from dcuid.h */
                cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
                            (*subAssoc)->params, dcmAllStorageSOPClassUIDs,
                            numberOfAllDcmStorageSOPClassUIDs,
                            transferSyntaxes, numTransferSyntaxes);
            }
        }
        if (cond.good()) cond = ASC_acknowledgeAssociation(*subAssoc);
        if (cond.bad())
        {
            cond = ASC_dropAssociation(*subAssoc);
            cond = ASC_destroyAssociation(subAssoc);
        }
    }
    else
    {
        T_DIMSE_Message     msg;
        T_ASC_PresentationContextID presID;

        if (!ASC_dataWaiting(*subAssoc, 0)) /* just in case */
            return;

        OFCondition cond = DIMSE_receiveCommand(*subAssoc, DIMSE_BLOCKING, 30,
                                                &presID, &msg, NULL);
        if (cond == EC_Normal)
        {
            switch (msg.CommandField)
            {
            case DIMSE_C_STORE_RQ:
                cond = storeSCP(*subAssoc, &msg, presID);
                break;
            default:
                cond = DIMSE_BADCOMMANDTYPE;
                break;
            }
        }
        /* clean up on association termination */
        if (cond == DUL_PEERREQUESTEDRELEASE)
        {
            cond = ASC_acknowledgeRelease(*subAssoc);
            cond = ASC_dropSCPAssociation(*subAssoc);
            cond = ASC_destroyAssociation(subAssoc);
            return;
        }
        else if (cond == DUL_PEERABORTEDASSOCIATION)
        {
        }
        else if (cond != EC_Normal)
        {
            std::stringstream stream;
            stream << "DIMSE failure (aborting sub-association): " << cond.text();
            cond = ASC_abortAssociation(*subAssoc);
            throw dopamine::ExceptionPACS(stream.str());
        }

        if (cond != EC_Normal)
        {
            cond = ASC_dropAssociation(*subAssoc);
            cond = ASC_destroyAssociation(subAssoc);
        }
    }
}

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    dopamine::GetSCP * getscp =
            new dopamine::GetSCP(NULL, T_ASC_PresentationContextID(), NULL);

    BOOST_REQUIRE_EQUAL(getscp != NULL, true);

    delete getscp;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Execute Get
 */
struct TestDataOK02
{
    DcmDataset * dataset;

    TestDataOK02()
    {
        // Start NetworkPACS (create and launch thread)
        boost::thread networkThread(launchNetwork);
        sleep(1); // Wait network initialisation

        // Create Dataset To Move
        dataset = new DcmDataset();
        dataset->putAndInsertOFStringArray(DCM_PatientName, "Doe^Jane");
        dataset->putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "PATIENT");
    }

    ~TestDataOK02()
    {
        delete dataset;

        sleep(1);
        terminateNetwork();
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK02)
{
    std::string writingport(getenv("DOPAMINE_TEST_WRITINGPORT"));

    /* initialize network, i.e. create an instance of T_ASC_Network*. */
    T_ASC_Network * networkSCU;
    OFCondition condition = ASC_initializeNetwork(NET_REQUESTOR,
                                                  atoi(writingport.c_str()),
                                                  30, &networkSCU);
    BOOST_CHECK_EQUAL(condition.good(), true);

    /* initialize asscociation parameters, i.e. create an instance of
     * T_ASC_Parameters*. */
    T_ASC_Parameters * params;
    condition = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
    BOOST_CHECK_EQUAL(condition.good(), true);

    /* sets this application's title and the called application's title
     * in the params */
    condition = ASC_setAPTitles(params, "LOCAL", "REMOTE", NULL);
    BOOST_CHECK_EQUAL(condition.good(), true);

    /* Figure out the presentation addresses and copy the corresponding
     * values into the association parameters.*/
    std::string localhost(128, '\0');
    gethostname(&localhost[0], localhost.size()-1);
    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));
    std::stringstream addressport;
    addressport << "localhost:" << listeningport;
    condition = ASC_setPresentationAddresses(params, localhost.c_str(),
                                             addressport.str().c_str());
    BOOST_CHECK_EQUAL(condition.good(), true);

    /* Add presentation contexts */
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

    /* create association, i.e. try to establish a network connection to another
     * DICOM application. This call creates an instance of T_ASC_Association*. */
    T_ASC_Association * association;
    condition = ASC_requestAssociation(networkSCU, params, &association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    /* find usable presentation context ID */
    T_ASC_PresentationContextID const presentation_id =
        ASC_findAcceptedPresentationContextID(association,
                                              UID_MRImageStorage);
    BOOST_CHECK_EQUAL(presentation_id != 0, true);

    T_ASC_PresentationContext pc;
    condition = ASC_findAcceptedPresentationContext(association->params,
                                                    presentation_id, &pc);
    BOOST_CHECK_EQUAL(presentation_id != 0, true);

    /* Prepare DIMSE data structures for issuing request */
    DIC_US const message_id = association->nextMsgID++;

    T_DIMSE_C_GetRQ * request = new T_DIMSE_C_GetRQ();
    memset(request, 0, sizeof(*request));
    request->MessageID = message_id;
    strcpy(request->AffectedSOPClassUID, UID_MRImageStorage);

    request->DataSetType = DIMSE_DATASET_PRESENT;
    request->Priority = DIMSE_PRIORITY_MEDIUM;

    T_DIMSE_C_GetRSP response;
    DcmDataset *detail = NULL;
    DcmDataset *rspIds = NULL;

    condition = DIMSE_getUser(association, presentation_id, request, dataset,
                              NULL /* callback */, NULL /* callbackData */,
                              DIMSE_BLOCKING, 30, networkSCU,
                              subOpCallback, NULL /* subopcallbackdata*/,
                              &response, &detail, &rspIds);
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
