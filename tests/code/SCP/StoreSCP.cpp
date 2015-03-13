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

#include <qt4/Qt/qstring.h>
#include <qt4/Qt/qstringlist.h>
#include <qt4/Qt/qprocess.h>

#include "SCP/StoreSCP.h"

/**
 * Pre-conditions:
 *     - we assume that NetworkPACS works correctly
 *
 *     - Following Environment variables should be defined
 *          * DOPAMINE_TEST_LISTENINGPORT
 *          * DOPAMINE_TEST_OUTPUTDIR
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
BOOST_AUTO_TEST_CASE(Run_StoreSCP)
{
    DcmDataset dataset;
    dataset.putAndInsertOFStringArray(DCM_PatientName, "Doe^John");
    dataset.putAndInsertOFStringArray(DCM_SOPClassUID, "1.2.840.10008.5.1.4.1.1.4");
    dataset.putAndInsertOFStringArray(DCM_Modality, "MR");
    dataset.putAndInsertOFStringArray(DCM_ImageType, "ORIGINAL\\PRIMARY\\OTHER");
    dataset.putAndInsertOFStringArray(DCM_SOPInstanceUID,
                                       "1.3.12.2.1107.5.2.36.40480.2013092014393692825160048");

    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));
    std::string outputdir(getenv("DOPAMINE_TEST_OUTPUTDIR"));

    std::stringstream stream;
    stream << outputdir << "/" << "JOHNDOE";

    // Create DICOM file
    DcmFileFormat fileformat(&dataset);
    OFCondition result = fileformat.saveFile(stream.str().c_str(),
                                             EXS_LittleEndianExplicit);
    BOOST_CHECK_EQUAL(result == EC_Normal, true);

    std::stringstream com_string;
    com_string << "storescu -aet LOCAL -aec REMOTE "
               << "localhost " << listeningport << " " << stream.str();
    QString command_(com_string.str().c_str());

    QProcess *myProcess = new QProcess();
    myProcess->start(command_);
    myProcess->waitForFinished(5000);

    // Check results
    BOOST_CHECK_EQUAL(myProcess->exitCode(), 0);
    BOOST_CHECK_EQUAL(myProcess->exitStatus(), QProcess::NormalExit);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardOutput().length(), 0);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardError().length(), 0);

    boost::filesystem::remove(boost::filesystem::path(stream.str().c_str()));
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Execute Store with SOPInstanceUID already register
 */
BOOST_AUTO_TEST_CASE(SOPInstanceUID_Already_Register)
{
    DcmDataset dataset;
    dataset.putAndInsertOFStringArray(DCM_PatientName, "Doe^John");
    dataset.putAndInsertOFStringArray(DCM_SOPClassUID, "1.2.840.10008.5.1.4.1.1.4");
    dataset.putAndInsertOFStringArray(DCM_Modality, "MR");
    dataset.putAndInsertOFStringArray(DCM_ImageType, "ORIGINAL\\PRIMARY\\OTHER");
    dataset.putAndInsertOFStringArray(DCM_SOPInstanceUID,
                                       "1.3.12.2.1107.5.2.36.40480.2013092014393692825160048");

    std::string listeningport(getenv("DOPAMINE_TEST_LISTENINGPORT"));
    std::string outputdir(getenv("DOPAMINE_TEST_OUTPUTDIR"));

    std::stringstream stream;
    stream << outputdir << "/" << "JOHNDOE";

    // Create DICOM file
    DcmFileFormat fileformat(&dataset);
    OFCondition result = fileformat.saveFile(stream.str().c_str(),
                                             EXS_LittleEndianExplicit);
    BOOST_CHECK_EQUAL(result == EC_Normal, true);

    std::stringstream com_string;
    com_string << "storescu -aet LOCAL -aec REMOTE "
               << "localhost " << listeningport << " " << stream.str();
    QString command_(com_string.str().c_str());

    QProcess *myProcess = new QProcess();
    myProcess->start(command_);
    myProcess->waitForFinished(5000);

    // Check results
    BOOST_CHECK_EQUAL(myProcess->exitCode(), 0);
    BOOST_CHECK_EQUAL(myProcess->exitStatus(), QProcess::NormalExit);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardOutput().length(), 0);
    BOOST_CHECK_EQUAL(myProcess->readAllStandardError().length(), 0);

    boost::filesystem::remove(boost::filesystem::path(stream.str().c_str()));
}
