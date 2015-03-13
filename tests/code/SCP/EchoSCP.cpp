/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#define BOOST_TEST_MODULE ModuleEchoSCP
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

#include "SCP/EchoSCP.h"

/**
 * Pre-conditions:
 *     - we assume that ConfigurationPACS works correctly
 *     - we assume that AuthenticatorCSV works correctly
 *
 *     - Following Environment variables should be defined
 *          * DOPAMINE_TEST_LISTENINGPORT
 *          * DOPAMINE_TEST_WRITINGPORT
 *          * DOPAMINE_TEST_CONFIG
 */

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Execute Echo
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    std::string writingport(getenv("DOPAMINE_TEST_WRITINGPORT"));

    T_ASC_Network * networkSCU;
    OFCondition condition = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR,
                                                  atoi(writingport.c_str()),
                                                  30, &networkSCU);
    BOOST_CHECK_EQUAL(condition.good(), true);

    T_ASC_Parameters * params;
    condition = ASC_createAssociationParameters(&params, ASC_MAXIMUMPDUSIZE);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_setAPTitles(params, "LOCAL", "REMOTE", NULL);
    BOOST_CHECK_EQUAL(condition.good(), true);

    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));

    std::stringstream addressport;
    addressport << "localhost:" << listeningport;
    condition = ASC_setPresentationAddresses(params, "localhost",
                                             addressport.str().c_str());
    BOOST_CHECK_EQUAL(condition.good(), true);

    typedef std::pair<std::string, std::vector<std::string> > PresentationContext;
    std::vector<PresentationContext> presentation_contexts;
    std::vector<std::string> tempvect = { UID_LittleEndianImplicitTransferSyntax };
    presentation_contexts.push_back(std::make_pair(UID_VerificationSOPClass,
                                                   tempvect));
    presentation_contexts.push_back(std::make_pair(UID_MRImageStorage,
                                                   tempvect));
    presentation_contexts.push_back(std::make_pair(UID_EnhancedMRImageStorage,
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

        context_id += 2;
    }

    T_ASC_Association * association;
    condition = ASC_requestAssociation(networkSCU, params, &association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    DIC_US const message_id = association->nextMsgID++;
    DIC_US status;
    DcmDataset *detail = NULL;
    // FIXME: block mode and timeout
    condition = DIMSE_echoUser(association, message_id, DIMSE_NONBLOCKING, 30,
                               &status, &detail);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_dropAssociation(association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_destroyAssociation(&association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_dropNetwork(&networkSCU);
    BOOST_CHECK_EQUAL(condition.good(), true);
}
