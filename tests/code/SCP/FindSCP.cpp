/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#define BOOST_TEST_MODULE ModuleFindSCP
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "SCP/FindSCP.h"

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
    dopamine::FindSCP * findscp =
            new dopamine::FindSCP(NULL, T_ASC_PresentationContextID(), NULL);

    BOOST_REQUIRE_EQUAL(findscp != NULL, true);

    delete findscp;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Execute Find
 */
BOOST_AUTO_TEST_CASE(TEST_OK_02)
{
    DcmDataset dataset;
    dataset.putAndInsertOFStringArray(DCM_PatientName, "Doe^Jane");
    dataset.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "PATIENT");

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

    T_DIMSE_C_FindRQ * request = new T_DIMSE_C_FindRQ();
    memset(request, 0, sizeof(*request));
    request->MessageID = message_id;
    strcpy(request->AffectedSOPClassUID, UID_MRImageStorage);

    request->DataSetType = DIMSE_DATASET_PRESENT;
    request->Priority = DIMSE_PRIORITY_MEDIUM;

    T_DIMSE_C_FindRSP response;
    DcmDataset *detail = NULL;

    condition = DIMSE_findUser(association, presentation_id, request,
                               &dataset, NULL, NULL, DIMSE_BLOCKING, 30,
                               &response, &detail);

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

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: No QueryRetrieveLevel
 */
BOOST_AUTO_TEST_CASE(TEST_KO_01)
{
    DcmDataset dataset;
    dataset.putAndInsertOFStringArray(DCM_PatientName, "Doe^Jane");

    std::string writingport(getenv("DOPAMINE_TEST_WRITINGPORT"));

    T_ASC_Network * networkSCU;
    OFCondition condition = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR,
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

    T_DIMSE_C_FindRQ * request = new T_DIMSE_C_FindRQ();
    memset(request, 0, sizeof(*request));
    request->MessageID = message_id;
    strcpy(request->AffectedSOPClassUID, UID_MRImageStorage);

    request->DataSetType = DIMSE_DATASET_PRESENT;
    request->Priority = DIMSE_PRIORITY_MEDIUM;

    T_DIMSE_C_FindRSP response;
    DcmDataset * detail = NULL;

    condition = DIMSE_findUser(association, presentation_id, request, &dataset,
                               NULL, NULL, DIMSE_BLOCKING, 30,
                               &response, &detail);

    BOOST_CHECK_EQUAL(condition.good(), true);
    BOOST_CHECK_EQUAL(response.DimseStatus,
                      STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass);
    BOOST_CHECK_EQUAL(detail != NULL, true);

    condition = ASC_releaseAssociation(association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_destroyAssociation(&association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_dropNetwork(&networkSCU);
    BOOST_CHECK_EQUAL(condition.good(), true);
}
