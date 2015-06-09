/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#define BOOST_TEST_MODULE ModuleConfigurationPACS
#include <boost/test/unit_test.hpp>

#include "core/ConfigurationPACS.h"
#include "core/ExceptionPACS.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    dopamine::ConfigurationPACS::get_instance();
    dopamine::ConfigurationPACS::delete_instance();
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Parsing configuration file
 */
 struct TestDataOK02
{
    std::string filename;
 
    TestDataOK02()
    {
        filename = "./tmp_test_moduleConfigurationPACS.ini";
        
        std::ofstream myfile;
        myfile.open(filename);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "allowed_peers=*\n";
        myfile << "port=11112\n";
        myfile << "[database]\n";
        myfile << "hostname=localhost\n";
        myfile << "port=27017\n";
        myfile << "dbname=pacs\n";
        myfile << "[authenticator]\n";
        myfile << "type=CSV\n";
        myfile << "filepath=./authentest.csv\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=LANGUEDOC,LOCAL\n";
        myfile << "LANGUEDOC=languedoc:11113\n";
        myfile << "LOCAL=vexin:11112\n";
        myfile.close();
    }
 
    ~TestDataOK02()
    {
        remove(filename.c_str());
        dopamine::ConfigurationPACS::delete_instance();
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK02)
{
    dopamine::ConfigurationPACS& confpacs = dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    BOOST_CHECK_EQUAL(confpacs.get_value("dicom.port"), "11112");
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Retrieve value
 */

BOOST_FIXTURE_TEST_CASE(TEST_OK_03, TestDataOK02)
{
    dopamine::ConfigurationPACS& confpacs = dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    BOOST_CHECK_EQUAL(confpacs.get_value("dicom.port"), "11112");
    BOOST_CHECK_EQUAL(confpacs.get_value("database", "port"), "27017");
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: Contains value
 */

BOOST_FIXTURE_TEST_CASE(TEST_OK_04, TestDataOK02)
{
    dopamine::ConfigurationPACS& confpacs = dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    BOOST_CHECK_EQUAL(confpacs.has_value("dicom.port"), true);
    BOOST_CHECK_EQUAL(confpacs.has_value("database", "port"), true);
    
    BOOST_CHECK_EQUAL(confpacs.has_value("badsection.port"), false);
    BOOST_CHECK_EQUAL(confpacs.has_value("database", "badfield"), false);
}

/*************************** TEST OK 05 *******************************/
/**
 * Nominal test case: testing peerInAETitle Everybody Allowed ('*')
 */

BOOST_FIXTURE_TEST_CASE(TEST_OK_05, TestDataOK02)
{
    dopamine::ConfigurationPACS& confpacs = dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("VALUE"), true);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("LOCAL"), true);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("NOERROR"), true);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle(""), true);
}
 
/*************************** TEST OK 06 *******************************/
/**
 * Nominal test case: testing peerInAETitle Specific user Allowed
 */
 struct TestDataOK06
{
    std::string filename;
 
    TestDataOK06()
    {
        filename = "./tmp_test_moduleConfigurationPACS.ini";
        
        std::ofstream myfile;
        myfile.open(filename);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "allowed_peers=USER1,USER3\n";
        myfile << "port=11112\n";
        myfile << "[database]\n";
        myfile << "hostname=localhost\n";
        myfile << "port=27017\n";
        myfile << "dbname=pacs\n";
        myfile << "[authenticator]\n";
        myfile << "type=CSV\n";
        myfile << "filepath=./authentest.csv\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=LANGUEDOC,LOCAL\n";
        myfile << "LANGUEDOC=languedoc:11113\n";
        myfile << "LOCAL=vexin:11112\n";
        myfile.close();
    }
 
    ~TestDataOK06()
    {
        remove(filename.c_str());
        dopamine::ConfigurationPACS::delete_instance();
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_06, TestDataOK06)
{
    dopamine::ConfigurationPACS& confpacs = dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("USER1"), true);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("USER3"), true);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle("USER2"), false);
    BOOST_CHECK_EQUAL(confpacs.peer_in_aetitle(""), false);
}

/*************************** TEST OK 07 *******************************/
/**
 * Nominal test case: testing peerForAETitle
 */

BOOST_FIXTURE_TEST_CASE(TEST_OK_07, TestDataOK06)
{
    dopamine::ConfigurationPACS& confpacs = dopamine::ConfigurationPACS::get_instance();
    confpacs.parse(filename);
    
    std::string address;
    BOOST_CHECK_EQUAL(confpacs.peer_for_aetitle("LANGUEDOC", address), true);
    BOOST_CHECK_EQUAL(address, "languedoc:11113");
    
    address = "value";
    BOOST_CHECK_EQUAL(confpacs.peer_for_aetitle("ERROR", address), false);
    BOOST_CHECK_EQUAL(address, "");
    
    address = "value";
    BOOST_CHECK_EQUAL(confpacs.peer_for_aetitle("", address), false);
    BOOST_CHECK_EQUAL(address, "");
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Parsing failure => Unknown file
 */
BOOST_AUTO_TEST_CASE(TEST_KO_01)
{
    dopamine::ConfigurationPACS& confpacs = dopamine::ConfigurationPACS::get_instance();
    BOOST_REQUIRE_THROW(confpacs.parse("badfilename"),
                        dopamine::ExceptionPACS);
    dopamine::ConfigurationPACS::delete_instance();
}
 
/*************************** TEST KO 02 *******************************/
/**
 * Error test case: Parsing failure => Missing mandatory field dicom.allowed_peers
 */
 struct TestDataKO02
{
    std::string filename;
 
    TestDataKO02()
    {
        filename = "./tmp_test_moduleConfigurationPACS.ini";
        
        std::ofstream myfile;
        myfile.open(filename);
        myfile << "[dicom]\n";
        myfile << "storage_path=./temp_dir\n";
        myfile << "port=11112\n";
        myfile << "[database]\n";
        myfile << "hostname=localhost\n";
        myfile << "port=27017\n";
        myfile << "dbname=pacs\n";
        myfile << "[authenticator]\n";
        myfile << "type=CSV\n";
        myfile << "filepath=./authentest.csv\n";
        myfile << "[listAddressPort]\n";
        myfile << "allowed=LANGUEDOC,LOCAL\n";
        myfile << "LANGUEDOC=languedoc:11113\n";
        myfile << "LOCAL=vexin:11112\n";
        myfile.close();
    }
 
    ~TestDataKO02()
    {
        remove(filename.c_str());
        dopamine::ConfigurationPACS::delete_instance();
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_KO_02, TestDataKO02)
{
    dopamine::ConfigurationPACS& confpacs = dopamine::ConfigurationPACS::get_instance();
    BOOST_REQUIRE_THROW(confpacs.parse(filename),
                        dopamine::ExceptionPACS);
}
