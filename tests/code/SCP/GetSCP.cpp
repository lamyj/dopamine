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

#include <qt4/Qt/qstring.h>
#include <qt4/Qt/qstringlist.h>
#include <qt4/Qt/qprocess.h>

#include "SCP/GetSCP.h"

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
    dopamine::GetSCP * getscp =
            new dopamine::GetSCP(NULL, T_ASC_PresentationContextID(), NULL);

    BOOST_REQUIRE_EQUAL(getscp != NULL, true);

    delete getscp;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Send C-GET
 */
BOOST_AUTO_TEST_CASE(Run_GetSCU)
{
    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));
    std::string outputdir(getenv("DOPAMINE_TEST_OUTPUTDIR"));

    // Check if getscu binary file exists
    BOOST_CHECK_EQUAL(boost::filesystem::exists(boost::filesystem::path("./tools/getscu")), true);

    std::stringstream com_string;
    com_string << "./getscu -aet LOCAL -aec REMOTE -P -k 0010,0010=\"Doe^Jane\" -k 0008,0052=\"PATIENT\" -od "
               << outputdir << " localhost " << listeningport;
    QString command_(com_string.str().c_str());

    QProcess *myProcess = new QProcess();
    myProcess->setWorkingDirectory(QString("./tools"));
    myProcess->start(command_);
    myProcess->waitForFinished(5000);

    // Check results
    BOOST_CHECK_EQUAL(myProcess->exitCode(), 0);
    BOOST_CHECK_EQUAL(myProcess->exitStatus(), QProcess::NormalExit);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardOutput().length(), 0);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardError().length(), 0);

    bool filecreate = false;
    // Check dataset
    typedef boost::filesystem::directory_iterator Iterator;
    for(Iterator it(outputdir); it != Iterator(); ++it)
    {
        if(! boost::filesystem::is_directory( (*it) ) )
        {
            DcmFileFormat fileformat;
            OFCondition condition = fileformat.loadFile((*it).path().c_str());
            DcmDataset * dataset = fileformat.getDataset();

            BOOST_CHECK_EQUAL(condition == EC_Normal, true);

            OFString patient_name;
            condition = dataset->findAndGetOFStringArray(DCM_PatientName, patient_name);
            BOOST_CHECK_EQUAL(condition == EC_Normal, true);
            BOOST_CHECK_EQUAL(std::string(patient_name.c_str()), "Doe^Jane");

            filecreate = true;
        }
    }
    BOOST_CHECK_EQUAL(filecreate, true);
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: No QueryRetrieveLevel
 */
BOOST_AUTO_TEST_CASE(Missing_QueryRetrieveLevel)
{
    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));
    std::string outputdir(getenv("DOPAMINE_TEST_OUTPUTDIR"));

    // Check if getscu binary file exists
    BOOST_CHECK_EQUAL(boost::filesystem::exists(boost::filesystem::path("./tools/getscu")), true);

    std::stringstream com_string;
    com_string << "./getscu -aet LOCAL -aec REMOTE -P -k 0010,0010=\"Doe^Jane\" -od "
               << outputdir << " localhost " << listeningport;
    QString command_(com_string.str().c_str());

    QProcess *myProcess = new QProcess();
    myProcess->setWorkingDirectory(QString("./tools"));
    myProcess->start(command_);
    myProcess->waitForFinished(5000);

    // Check results
    BOOST_CHECK_EQUAL(myProcess->exitCode(), 0);
    BOOST_CHECK_EQUAL(myProcess->exitStatus(), QProcess::NormalExit);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardOutput().length(), 0);
    BOOST_CHECK_GT(myProcess->readAllStandardError().length(), 0);
}

/*************************** TEST KO 02 *******************************/
/**
 * Error test case: No image location file
 */
BOOST_AUTO_TEST_CASE(No_PixelData)
{
    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));
    std::string outputdir(getenv("DOPAMINE_TEST_OUTPUTDIR"));

    // Check if getscu binary file exists
    BOOST_CHECK_EQUAL(boost::filesystem::exists(boost::filesystem::path("./tools/getscu")), true);

    std::stringstream com_string;
    com_string << "./getscu -aet LOCAL -aec REMOTE -P "
               << "-k 0020,000e=\"2.16.756.5.5.100.3611280983.20092.1364462499.1\" -k 0008,0052=\"STUDY\" -od "
               << outputdir << " localhost " << listeningport;
    QString command_(com_string.str().c_str());

    QProcess *myProcess = new QProcess();
    myProcess->setWorkingDirectory(QString("./tools"));
    myProcess->start(command_);
    myProcess->waitForFinished(5000);

    // Check results
    BOOST_CHECK_EQUAL(myProcess->exitCode(), 0);
    BOOST_CHECK_EQUAL(myProcess->exitStatus(), QProcess::NormalExit);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardOutput().length(), 0);
    BOOST_CHECK_GT(myProcess->readAllStandardError().length(), 0);
}
