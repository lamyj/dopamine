/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#define BOOST_TEST_MODULE ModuleStoreSCP
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include <qt4/Qt/qstring.h>
#include <qt4/Qt/qstringlist.h>
#include <qt4/Qt/qprocess.h>

#include "core/ConfigurationPACS.h"
#include "core/DBConnection.h"
#include "core/NetworkPACS.h"

#include "SCP/StoreSCP.h"

/**
 * Pre-conditions:
 *     - we assume that ConfigurationPACS works correctly
 *     - we assume that AuthenticatorNone works correctly
 */

const std::string NetworkConfFILE = "./tmp_test_ModuleStoreSCP_conf.ini";

void launchNetwork()
{
    research_pacs::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);

    // Get all indexes
    std::string indexlist =
        research_pacs::ConfigurationPACS::get_instance().GetValue("database.indexlist");
    std::vector<std::string> indexlistvect;
    boost::split(indexlistvect, indexlist, boost::is_any_of(";"));

    // Create and Initialize DB connection
    research_pacs::DBConnection::get_instance().Initialize
        (
            research_pacs::ConfigurationPACS::get_instance().GetValue("database.dbname"),
            research_pacs::ConfigurationPACS::get_instance().GetValue("database.hostname"),
            research_pacs::ConfigurationPACS::get_instance().GetValue("database.port"),
            indexlistvect
        );

    // Connect Database
    research_pacs::DBConnection::get_instance().connect();

    research_pacs::NetworkPACS& networkpacs =
            research_pacs::NetworkPACS::get_instance();
    networkpacs.run();

    research_pacs::ConfigurationPACS::delete_instance();
    research_pacs::NetworkPACS::delete_instance();
}

void terminateNetwork()
{
    // Call Terminate SCU
    QString command = "termscu";
    QStringList args;
    args << "localhost" << "11112";

    QProcess *myProcess = new QProcess();
    myProcess->start(command, args);
    myProcess->waitForFinished(5000);
}

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    research_pacs::StoreSCP * storescp =
            new research_pacs::StoreSCP(NULL, T_ASC_PresentationContextID(), NULL);

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
        myfile << "allowed=LANGUEDOC,LOCAL\n";
        myfile << "LANGUEDOC=languedoc:11113\n";
        myfile << "LOCAL=vexin:11112\n";
        myfile.close();

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

        remove(NetworkConfFILE.c_str());

        sleep(1);
        boost::filesystem::remove_all("./temp_dir");

        terminateNetwork();
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK02)
{
    OFCondition condition;

    T_ASC_Network * networkSCU;
    condition = ASC_initializeNetwork(NET_REQUESTOR, 11113, 30,
                                      &networkSCU);
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

    condition = ASC_setPresentationAddresses(params, localhost.c_str(),
                                             "localhost:11112");
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

    condition = ASC_releaseAssociation(association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_destroyAssociation(&association);
    BOOST_CHECK_EQUAL(condition.good(), true);

    condition = ASC_dropNetwork(&networkSCU);
    BOOST_CHECK_EQUAL(condition.good(), true);
}
