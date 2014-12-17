/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#define BOOST_TEST_MODULE ModuleStoreSCP
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "SCP/StoreSCP.h"
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

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    dopamine::StoreSCP * storescp =
            new dopamine::StoreSCP(NULL, T_ASC_PresentationContextID(), NULL);

    BOOST_REQUIRE_EQUAL(storescp != NULL, true);

    delete storescp;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Execute Store
 */
struct TestDataOK02
{
    DcmDataset * dataset;

    TestDataOK02()
    {
        // Start NetworkPACS (create and launch thread)
        boost::thread networkThread(launchNetwork);
        sleep(1); // Wait network initialisation

        // Create Dataset To store
        dataset = new DcmDataset();
        dataset->putAndInsertOFStringArray(DCM_PatientName, "Doe^John");
        dataset->putAndInsertOFStringArray(DCM_SOPClassUID, "1.2.840.10008.5.1.4.1.1.4");
        dataset->putAndInsertOFStringArray(DCM_Modality, "MR");
        dataset->putAndInsertOFStringArray(DCM_ImageType, "ORIGINAL\\PRIMARY\\OTHER");
        dataset->putAndInsertOFStringArray(DCM_SOPInstanceUID,
                                           "1.3.12.2.1107.5.2.36.40480.2013092014393692825160048");
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

    T_ASC_Network * networkSCU;
    OFCondition condition = ASC_initializeNetwork(NET_REQUESTOR,
                                                  atoi(writingport.c_str()),
                                                  30, &networkSCU);
    BOOST_CHECK_EQUAL(condition.good(), true);

    T_ASC_Parameters * params;
    condition = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_setAPTitles(params, "LOCAL", "REMOTE", NULL);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_setTransportLayerType(params, OFFalse);
    BOOST_CHECK_EQUAL(condition.good(), true);

    std::string localhost(128, '\0');
    gethostname(&localhost[0], localhost.size()-1);

    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));

    std::stringstream addressport;
    addressport << "localhost:" << listeningport;
    condition = ASC_setPresentationAddresses(params, localhost.c_str(),
                                             addressport.str().c_str());
    BOOST_CHECK_EQUAL(condition.good(), true);

    typedef std::pair<std::string, std::vector<std::string> > PresentationContext;
    std::vector<PresentationContext> presentation_contexts;
    std::vector<std::string> tempvect = { UID_LittleEndianExplicitTransferSyntax,
                                          UID_LittleEndianImplicitTransferSyntax,
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

    T_DIMSE_C_StoreRQ * request = new T_DIMSE_C_StoreRQ();
    memset(request, 0, sizeof(*request));
    request->MessageID = message_id;
    strcpy(request->AffectedSOPClassUID, UID_MRImageStorage);

    OFString sop_instance_uid;
    const_cast<DcmDataset*>(dataset)->findAndGetOFString(DCM_SOPInstanceUID,
                                                         sop_instance_uid);
    strcpy(request->AffectedSOPInstanceUID, sop_instance_uid.c_str());
    BOOST_CHECK_EQUAL(sop_instance_uid.size() != 0, true);

    request->DataSetType = DIMSE_DATASET_PRESENT;
    request->Priority = DIMSE_PRIORITY_MEDIUM;

    T_DIMSE_C_StoreRSP response;
    DcmDataset *detail = NULL;

    condition = DIMSE_storeUser(association, presentation_id, request,
                                NULL, dataset, NULL, NULL, DIMSE_NONBLOCKING,
                                30, &response, &detail, NULL, 0);
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

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Execute Store with SOPInstanceUID already register
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_03, TestDataOK02)
{
    std::string writingport(getenv("DOPAMINE_TEST_WRITINGPORT"));

    T_ASC_Network * networkSCU;
    OFCondition condition = ASC_initializeNetwork(NET_REQUESTOR,
                                                  atoi(writingport.c_str()),
                                                  30, &networkSCU);
    BOOST_CHECK_EQUAL(condition.good(), true);

    T_ASC_Parameters * params;
    condition = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_setAPTitles(params, "LOCAL", "REMOTE", NULL);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_setTransportLayerType(params, OFFalse);
    BOOST_CHECK_EQUAL(condition.good(), true);

    std::string localhost(128, '\0');
    gethostname(&localhost[0], localhost.size()-1);

    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));

    std::stringstream addressport;
    addressport << "localhost:" << listeningport;
    condition = ASC_setPresentationAddresses(params, localhost.c_str(),
                                             addressport.str().c_str());
    BOOST_CHECK_EQUAL(condition.good(), true);

    typedef std::pair<std::string, std::vector<std::string> > PresentationContext;
    std::vector<PresentationContext> presentation_contexts;
    std::vector<std::string> tempvect = { UID_LittleEndianExplicitTransferSyntax,
                                          UID_LittleEndianImplicitTransferSyntax,
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

    T_DIMSE_C_StoreRQ * request = new T_DIMSE_C_StoreRQ();
    memset(request, 0, sizeof(*request));
    request->MessageID = message_id;
    strcpy(request->AffectedSOPClassUID, UID_MRImageStorage);

    OFString sop_instance_uid;
    const_cast<DcmDataset*>(dataset)->findAndGetOFString(DCM_SOPInstanceUID,
                                                         sop_instance_uid);
    strcpy(request->AffectedSOPInstanceUID, sop_instance_uid.c_str());
    BOOST_CHECK_EQUAL(sop_instance_uid.size() != 0, true);

    request->DataSetType = DIMSE_DATASET_PRESENT;
    request->Priority = DIMSE_PRIORITY_MEDIUM;

    T_DIMSE_C_StoreRSP response;
    DcmDataset *detail = NULL;

    condition = DIMSE_storeUser(association, presentation_id, request,
                                NULL, dataset, NULL, NULL, DIMSE_NONBLOCKING,
                                30, &response, &detail, NULL, 0);
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
