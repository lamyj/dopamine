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

#include <qt4/Qt/qstring.h>
#include <qt4/Qt/qstringlist.h>
#include <qt4/Qt/qprocess.h>

#include "SCP/FindSCP.h"

/**
 * Pre-conditions:
 *     - we assume that NetworkPACS works correctly
 *
 *     - Following Environment variables should be defined
 *          * DOPAMINE_TEST_LISTENINGPORT
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
BOOST_AUTO_TEST_CASE(Run_FindSCU)
{
    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));

    std::stringstream com_string;
    com_string << "findscu -aet LOCAL -aec REMOTE -P -k 0010,0010=\"Doe^Jane\" -k 0008,0052=\"PATIENT\" "
               << " localhost " << listeningport;
    QString command_(com_string.str().c_str());

    QProcess *myProcess = new QProcess();
    myProcess->start(command_);
    myProcess->waitForFinished(5000);

    // Check results
    BOOST_CHECK_EQUAL(myProcess->exitCode(), 0);
    BOOST_CHECK_EQUAL(myProcess->exitStatus(), QProcess::NormalExit);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardOutput().length(), 0);

    auto erroroutput = myProcess->readAllStandardError();
    BOOST_CHECK_GT(erroroutput.length(), 0);

    std::string output;
    for (int i = 0; i < erroroutput.length(); ++i)
    {
        output.push_back(erroroutput.at(i));
    }

    BOOST_CHECK_EQUAL(output.find("Find Response: 1") != std::string::npos, true);
    BOOST_CHECK_EQUAL(output.find("(0010,0010) PN [Doe^Jane]") != std::string::npos, true);
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: No QueryRetrieveLevel
 */
BOOST_AUTO_TEST_CASE(TEST_KO_01)
{
    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));

    std::stringstream com_string;
    com_string << "findscu -aet LOCAL -aec REMOTE -P -k 0010,0010=\"Doe^Jane\" "
               << " localhost " << listeningport;
    QString command_(com_string.str().c_str());

    QProcess *myProcess = new QProcess();
    myProcess->start(command_);
    myProcess->waitForFinished(5000);

    // Check results
    BOOST_CHECK_EQUAL(myProcess->exitCode(), 0);
    BOOST_CHECK_EQUAL(myProcess->exitStatus(), QProcess::NormalExit);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardOutput().length(), 0);

    auto erroroutput = myProcess->readAllStandardError();
    BOOST_CHECK_GT(erroroutput.length(), 0);

    std::string output;
    for (int i = 0; i < erroroutput.length(); ++i)
    {
        output.push_back(erroroutput.at(i));
    }

    BOOST_CHECK_EQUAL(output.find("(0000,0901) AT (0008,0052)") != std::string::npos, true);
    BOOST_CHECK_EQUAL(output.find("(0000,0902) LO [Tag not found ]") != std::string::npos, true);
}
