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

#include <qt4/Qt/qstring.h>
#include <qt4/Qt/qstringlist.h>
#include <qt4/Qt/qprocess.h>

#include "SCP/EchoSCP.h"

/**
 * Pre-conditions:
 *     - we assume that NetworkPACS works correctly
 *
 *     - Following Environment variables should be defined
 *          * DOPAMINE_TEST_LISTENINGPORT
 */

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor/Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::EchoSCP * echoscp =
            new dopamine::EchoSCP(NULL, T_ASC_PresentationContextID(), NULL);

    BOOST_REQUIRE_EQUAL(echoscp != NULL, true);

    delete echoscp;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Execute Echo
 */
BOOST_AUTO_TEST_CASE(Run_EchoSCU)
{
    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));

    std::stringstream com_string;
    com_string << "echoscu localhost " << listeningport;
    QString command_(com_string.str().c_str());

    QProcess *myProcess = new QProcess();
    myProcess->start(command_);
    myProcess->waitForFinished(5000);

    // Check results
    BOOST_CHECK_EQUAL(myProcess->exitCode(), 0);
    BOOST_CHECK_EQUAL(myProcess->exitStatus(), QProcess::NormalExit);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardOutput().length(), 0);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardError().length(), 0);
}
