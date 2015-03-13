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

std::string const configfile = "./tmp_test_moduleNetworkPACS_conf.ini";

void create_configuration_file(std::string const & authenticatortype)
{
    std::string server(getenv("TEST_LDAP_SERVER"));
    std::string base(getenv("TEST_LDAP_BASE"));
    std::string bind(getenv("TEST_LDAP_BIND"));

    // Create Configuration file
    std::ofstream myfile;
    myfile.open(configfile);
    myfile << "[dicom]\n";
    myfile << "storage_path=./temp_dir\n";
    myfile << "allowed_peers=*\n";
    myfile << "port=11114\n";
    myfile << "[database]\n";
    myfile << "hostname=localhost\n";
    myfile << "port=27017\n";
    myfile << "dbname=pacs\n";
    myfile << "[authenticator]\n";
    myfile << "type=" << authenticatortype << "\n";
    myfile << "# path of the authenticator file (only for type = CSV)\n";
    myfile << "filepath=./authentest.csv\n";
    myfile << "# LDAP Server Address (only for type = LDAP)\n";
    myfile << "ldap_server=" << server << "\n";
    myfile << "# User name for LDAP binding (only for type = LDAP)\n";
    myfile << "ldap_bind_user=" << bind << "\n";
    myfile << "# Base of search (only for type = LDAP)\n";
    myfile << "ldap_base=" << base << "\n";
    myfile << "# Request filter (only for type = LDAP)\n";
    myfile << "ldap_filter=(&(uid=%user)(memberof=cn=FLI-IAM,ou=Labo,dc=ipbrech,dc=local))\n";
    myfile << "[listAddressPort]\n";
    myfile << "allowed=LANGUEDOC,LOCAL\n";
    myfile << "LANGUEDOC=languedoc:11113\n";
    myfile << "LOCAL=vexin:11114\n";
    myfile.close();
}

void terminateNetwork()
{
    // Call Terminate SCU
    QString command = "termscu";
    QStringList args;
    args << "localhost" << "11114";

    QProcess *myProcess = new QProcess();
    myProcess->start(command, args);
    myProcess->waitForFinished(5000);
}

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor (None)
 */
 struct TestDataOK01
{
    TestDataOK01()
    {
        create_configuration_file("None");
        dopamine::ConfigurationPACS::get_instance().Parse(configfile);
    }
 
    ~TestDataOK01()
    {
        remove(configfile.c_str());
        dopamine::ConfigurationPACS::delete_instance();
        dopamine::NetworkPACS::delete_instance();
    }
};

BOOST_FIXTURE_TEST_CASE(Constructor_None, TestDataOK01)
{
    dopamine::NetworkPACS& networkpacs = dopamine::NetworkPACS::get_instance();
    BOOST_CHECK_EQUAL(networkpacs.get_network() != NULL, true);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Constructor (CSV)
 */
 struct TestDataOK02
{
    std::string csvfile;

    TestDataOK02()
    {
        create_configuration_file("CSV");
        dopamine::ConfigurationPACS::get_instance().Parse(configfile);

        csvfile = "./authentest.csv";

        std::ofstream myfile;
        myfile.open(csvfile);
        myfile << "user1 password1\n";
        myfile << "user2 password2\n";
        myfile.close();
    }

    ~TestDataOK02()
    {
        remove(configfile.c_str());
        remove(csvfile.c_str());
        dopamine::ConfigurationPACS::delete_instance();
        dopamine::NetworkPACS::delete_instance();
    }
};

BOOST_FIXTURE_TEST_CASE(Constructor_CSV, TestDataOK02)
{
    dopamine::NetworkPACS& networkpacs = dopamine::NetworkPACS::get_instance();
    BOOST_CHECK_EQUAL(networkpacs.get_network() != NULL, true);
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Constructor (LDAP)
 */
 struct TestDataOK03
{
    TestDataOK03()
    {
        create_configuration_file("LDAP");
        dopamine::ConfigurationPACS::get_instance().Parse(configfile);
    }

    ~TestDataOK03()
    {
        remove(configfile.c_str());
        dopamine::ConfigurationPACS::delete_instance();
        dopamine::NetworkPACS::delete_instance();
    }
};

BOOST_FIXTURE_TEST_CASE(Constructor_LDAP, TestDataOK03)
{
    dopamine::NetworkPACS& networkpacs = dopamine::NetworkPACS::get_instance();
    BOOST_CHECK_EQUAL(networkpacs.get_network() != NULL, true);
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: Run
 */

BOOST_FIXTURE_TEST_CASE(Run_forceStop, TestDataOK01)
{
    dopamine::NetworkPACS& networkpacs = dopamine::NetworkPACS::get_instance();
    networkpacs.force_stop();
    networkpacs.set_timeout(1);
    networkpacs.run();
    BOOST_CHECK_EQUAL(networkpacs.get_network() != NULL, true);
}

/*************************** TEST OK 05 *******************************/
/**
 * Nominal test case: Run and send Shutdown request
 */

BOOST_FIXTURE_TEST_CASE(Shutdown_Request, TestDataOK01)
{
    dopamine::NetworkPACS& networkpacs = dopamine::NetworkPACS::get_instance();

    // Stop NetworkPACS after 1second
    boost::thread stopThread(terminateNetwork);

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
    TestDataKO01()
    {
        create_configuration_file("BADVALUE");
        dopamine::ConfigurationPACS::get_instance().Parse(configfile);
    }
 
    ~TestDataKO01()
    {
        remove(configfile.c_str());
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
    TestDataKO02()
    {
        // Create Configuration file
        std::ofstream myfile;
        myfile.open(configfile);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "allowed_peers=*\n";
        myfile << "port=11112\n";               // WARNING: this port should be used
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
        
        dopamine::ConfigurationPACS::get_instance().Parse(configfile);
    }
 
    ~TestDataKO02()
    {
        remove(configfile.c_str());
        dopamine::ConfigurationPACS::delete_instance();
        dopamine::NetworkPACS::delete_instance();
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_KO_02, TestDataKO02)
{
    BOOST_REQUIRE_THROW(dopamine::NetworkPACS::get_instance(),
                        dopamine::ExceptionPACS);
}
