/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <qt4/Qt/qstring.h>
#include <qt4/Qt/qstringlist.h>
#include <qt4/Qt/qprocess.h>
#include <fstream>

#define BOOST_TEST_MODULE ModuleNetworkPACS
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

#include "core/ConfigurationPACS.h"
#include "core/ExceptionPACS.h"
#include "core/NetworkPACS.h"

/**
 * Pre-conditions: 
 *     - we assume that ConfigurationPACS works correctly
 *     - we assume that AuthenticatorNone works correctly
 */

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
 struct TestDataOK01
{
    std::string conffile;
 
    TestDataOK01()
    {
        // Create Configuration file
        conffile = "./tmp_test_moduleNetworkPACS_conf.ini";
        std::ofstream myfile;
        myfile.open(conffile);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "allowed_peers=*\n";
        myfile << "port=11112\n";
        myfile << "[database]\n";
        myfile << "hostname=localhost\n";
        myfile << "port=27017\n";
        myfile << "dbname=pacs\n";
        myfile << "[authenticator]\n";
        myfile << "type=None\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=LANGUEDOC,LOCAL\n";
        myfile << "LANGUEDOC=languedoc:11113\n";
        myfile << "LOCAL=vexin:11112\n";
        myfile.close();

        dopamine::ConfigurationPACS::get_instance().Parse(conffile);
    }
 
    ~TestDataOK01()
    {
        remove(conffile.c_str());
        dopamine::ConfigurationPACS::delete_instance();
        dopamine::NetworkPACS::delete_instance();
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_01, TestDataOK01)
{
    dopamine::NetworkPACS& networkpacs = dopamine::NetworkPACS::get_instance();
    BOOST_CHECK_EQUAL(networkpacs.get_network() != NULL, true);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Run
 */

BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK01)
{
    dopamine::NetworkPACS& networkpacs = dopamine::NetworkPACS::get_instance();
    networkpacs.force_stop();
    networkpacs.set_timeout(1);
    networkpacs.run();
    BOOST_CHECK_EQUAL(networkpacs.get_network() != NULL, true);
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Run and send Shutdown request
 */
void processTerminate()
{
    // Wait until Networkpacs started
    sleep(1);

    // Call Terminate SCU
    QString command = "termscu";
    QStringList args;
    args << "localhost" << "11112";

    QProcess *myProcess = new QProcess();
    myProcess->start(command, args);
}

BOOST_FIXTURE_TEST_CASE(TEST_OK_03, TestDataOK01)
{
    dopamine::NetworkPACS& networkpacs = dopamine::NetworkPACS::get_instance();

    // Stop NetworkPACS after 1second
    boost::thread stopThread(processTerminate);

    // Start NetworkPACS (stopped by another thread)
    networkpacs.run();
    BOOST_CHECK_EQUAL(networkpacs.get_network() != NULL, true);
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Bad authenticator type
 */
 struct TestDataKO01
{
    std::string conffile;
 
    TestDataKO01()
    {
        // Create Configuration file
        conffile = "./tmp_test_moduleNetworkPACS_conf.ini";
        std::ofstream myfile;
        myfile.open(conffile);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "allowed_peers=*\n";
        myfile << "port=11112\n";
        myfile << "[database]\n";
        myfile << "hostname=localhost\n";
        myfile << "port=27017\n";
        myfile << "dbname=pacs\n";
        myfile << "[authenticator]\n";
        myfile << "type=BADVALUE\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=LANGUEDOC,LOCAL\n";
        myfile << "LANGUEDOC=languedoc:11113\n";
        myfile << "LOCAL=vexin:11112\n";
        myfile.close();
        
        dopamine::ConfigurationPACS::get_instance().Parse(conffile);
    }
 
    ~TestDataKO01()
    {
        remove(conffile.c_str());
        dopamine::ConfigurationPACS::delete_instance();
        dopamine::NetworkPACS::delete_instance();
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_KO_01, TestDataKO01)
{
    BOOST_REQUIRE_THROW(dopamine::NetworkPACS::get_instance(),
                        dopamine::ExceptionPACS);
}

/*************************** TEST KO 02 *******************************/
/**
 * Error test case: Bad Port
 */
 struct TestDataKO02
{
    std::string conffile;
 
    TestDataKO02()
    {
        // Create Configuration file
        conffile = "./tmp_test_moduleNetworkPACS_conf.ini";
        std::ofstream myfile;
        myfile.open(conffile);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "allowed_peers=*\n";
        myfile << "port=1\n";               // WARNING: this port should be used
        myfile << "[database]\n";
        myfile << "hostname=localhost\n";
        myfile << "port=27017\n";
        myfile << "dbname=pacs\n";
        myfile << "[authenticator]\n";
        myfile << "type=None\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=LANGUEDOC,LOCAL\n";
        myfile << "LANGUEDOC=languedoc:11113\n";
        myfile << "LOCAL=vexin:11112\n";
        myfile.close();
        
        dopamine::ConfigurationPACS::get_instance().Parse(conffile);
    }
 
    ~TestDataKO02()
    {
        remove(conffile.c_str());
        dopamine::ConfigurationPACS::delete_instance();
        dopamine::NetworkPACS::delete_instance();
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_KO_02, TestDataKO02)
{
    BOOST_REQUIRE_THROW(dopamine::NetworkPACS::get_instance(),
                        dopamine::ExceptionPACS);
}
