/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleQido_rs
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <string>

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcistrmb.h>

#include "core/ConfigurationPACS.h"
#include "services/ServicesTools.h"
#include "services/webservices/Stow_rs.h"
#include "services/webservices/WebServiceException.h"

std::string const UNIQUE_SOP_INSTANCE_UID_04 = "1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489112";
std::string const UNIQUE_SOP_INSTANCE_UID_05 = "1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489113";
std::string const UNIQUE_SOP_INSTANCE_UID_06 = "1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489114";
std::string const UNIQUE_SOP_INSTANCE_UID_07 = "1.2.276.0.7230010.3.1.4.8323329.7922.1432822447.489240";
std::string const UNIQUE_SOP_INSTANCE_UID_08 = "1.2.276.0.7230010.3.1.4.8323329.7922.1432822447.489271";
std::string const UNIQUE_STUDY_INSTANCE_UID = "2.16.756.5.5.100.3611280983.14235.1379922643.24566";

struct TestDataRequest
{
    std::string path_info;
    std::string content_type;
    std::string boundary;
    mongo::DBClientConnection connection;
    std::string db_name;

    TestDataRequest()
    {
        std::string NetworkConfFILE(getenv("DOPAMINE_TEST_CONFIG"));
        dopamine::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);

        path_info = "/studies";
        boundary = "4EMVgTUJNpDOGeP";

        std::stringstream content_typestream;
        content_typestream << dopamine::services::CONTENT_TYPE
                           << dopamine::services::MIME_TYPE_MULTIPART_RELATED << "; type="
                           << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "; "
                           << dopamine::services::ATTRIBUT_BOUNDARY << boundary;
        content_type = content_typestream.str();

        // Create DataBase Connection
        dopamine::services::create_db_connection(connection, db_name);
    }

    ~TestDataRequest()
    {
        dopamine::ConfigurationPACS::delete_instance();
        sleep(1);
    }
};

struct TestDataRequestNotAllow : public TestDataRequest
{
    TestDataRequestNotAllow():
        TestDataRequest()
    {
        mongo::BSONObjBuilder builder;
        builder.appendRegex("00080018", "Unknown");
        mongo::BSONObj store_value = BSON("principal_name" << "root" <<
                                          "principal_type" << "" <<
                                          "service" << "Store" <<
                                          "dataset" << builder.obj());
        connection.update(db_name + ".authorization", BSON("service" << "Store"), store_value);

        mongo::BSONObjBuilder builder2;
        builder2 << "00080060" << "NotMR";
        mongo::BSONObj store_value2 = BSON("principal_name" << "not_me" <<
                                           "principal_type" << "" <<
                                           "service" << "Store" <<
                                           "dataset" << builder2.obj());
        connection.insert(db_name + ".authorization",
                           store_value2);

        if (connection.getLastError(db_name, true) != "")
        {
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS("An error occurred while storing object"));
        }
    }

    ~TestDataRequestNotAllow()
    {
        connection.remove(db_name + ".authorization", BSON("principal_name" << "not_me"));

        mongo::BSONObj store_value = BSON("principal_name" << "" <<
                                          "principal_type" << "" <<
                                          "service" << "Store" <<
                                          "dataset" << mongo::BSONObj());
        connection.update(db_name + ".authorization", BSON("service" << "Store"), store_value);
    }
};

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: qido_rs Accessors
 */
BOOST_FIXTURE_TEST_CASE(Accessors, TestDataRequest)
{
    std::stringstream dataset;
    dataset << content_type << "\n\n";
    dataset << "--" << boundary << "--";

    dopamine::services::Stow_rs stowrs(path_info, "", dataset.str());

    BOOST_CHECK_NE(stowrs.get_boundary(), "");
    BOOST_CHECK_EQUAL(stowrs.get_content_type(),
                      dopamine::services::MIME_TYPE_APPLICATION_DICOM);
    BOOST_CHECK_EQUAL(stowrs.get_status(), 200);
    BOOST_CHECK_EQUAL(stowrs.get_code(), "OK");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: stow_rs insert 1 dataset
 */
BOOST_FIXTURE_TEST_CASE(InsertOneDICOM, TestDataRequest)
{
    // Check SOP Instance UID not present in database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_07)), 0);

    std::string datasetfile(getenv("DOPAMINE_TEST_DICOMFILE_07"));

    std::stringstream dataset;
    dataset << content_type << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    // Open file
    std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
    BOOST_REQUIRE(file.is_open());
    // get length of file:
    int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));
    std::string output(length, '\0');
    // read data as a block:
    file.read (&output[0], output.size());
    // Close file
    file.close();

    dataset << output;
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    dopamine::services::Stow_rs stowrs(path_info, "", dataset.str());

    BOOST_CHECK_NE(stowrs.get_response(), "");

    boost::property_tree::ptree ptree;
    std::stringstream xmlstream;
    xmlstream << stowrs.get_response();
    boost::property_tree::read_xml(xmlstream, ptree);
    BOOST_CHECK_EQUAL(ptree.find("NativeDicomModel") != ptree.not_found(), true);

    // check mandatory tag
    BOOST_CHECK(xmlstream.str().find("tag=\"00081190\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081199\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081150\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081155\"") != std::string::npos);

    // check tag error is missing
    BOOST_CHECK(xmlstream.str().find("tag=\"00081198\"") == std::string::npos);

    // check values
    BOOST_CHECK(xmlstream.str().find("1.2.840.10008.5.1.4.1.1.4") !=
                std::string::npos);
    BOOST_CHECK(xmlstream.str().find(UNIQUE_SOP_INSTANCE_UID_07) !=
                std::string::npos);

    // Check into database
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_07)), 1);

    // remove dataset
    connection.remove(db_name + ".datasets", BSON("00080018.Value" <<
                                                  UNIQUE_SOP_INSTANCE_UID_07));
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: stow_rs insert 3 dataset
 */
BOOST_FIXTURE_TEST_CASE(InsertThreeDICOM, TestDataRequest)
{
    // Check SOP Instance UID not present in database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_04)), 0);
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_05)), 0);
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_06)), 0);

    std::vector<std::string> files = { std::string(getenv("DOPAMINE_TEST_DICOMFILE_04")),
                                       std::string(getenv("DOPAMINE_TEST_DICOMFILE_05")),
                                       std::string(getenv("DOPAMINE_TEST_DICOMFILE_06")) };

    std::stringstream dataset;
    dataset << content_type << "\n\n";
    for (std::string datasetfile : files)
    {
        dataset << "--" << boundary << "\n";
        dataset << dopamine::services::CONTENT_TYPE
                << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
        dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
                << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
        dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
                << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

        // Open file
        std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
        BOOST_REQUIRE(file.is_open());

        // get length of file:
        int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));

        std::string output(length, '\0');

        // read data as a block:
        file.read (&output[0], output.size());

        // Close file
        file.close();

        dataset << output;
        dataset << "\n" << "\n";
    }
    dataset << "--" << boundary << "--";

    dopamine::services::Stow_rs stowrs(path_info, "", dataset.str());

    BOOST_CHECK_NE(stowrs.get_response(), "");

    boost::property_tree::ptree ptree;
    std::stringstream xmlstream;
    xmlstream << stowrs.get_response();
    boost::property_tree::read_xml(xmlstream, ptree);
    BOOST_CHECK_EQUAL(ptree.find("NativeDicomModel") != ptree.not_found(), true);

    // check mandatory tag
    BOOST_CHECK(xmlstream.str().find("tag=\"00081190\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081199\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081150\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081155\"") != std::string::npos);

    // check tag error is missing
    BOOST_CHECK(xmlstream.str().find("tag=\"00081198\"") == std::string::npos);

    // check values
    BOOST_CHECK(xmlstream.str().find("1.2.840.10008.5.1.4.1.1.4") !=
                std::string::npos);
    BOOST_CHECK(xmlstream.str().find(UNIQUE_SOP_INSTANCE_UID_04) !=
                std::string::npos);
    BOOST_CHECK(xmlstream.str().find(UNIQUE_SOP_INSTANCE_UID_05) !=
                std::string::npos);
    BOOST_CHECK(xmlstream.str().find(UNIQUE_SOP_INSTANCE_UID_06) !=
                std::string::npos);

    // Check into database
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_04)), 1);
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_05)), 1);
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_06)), 1);

    // remove dataset
    connection.remove(db_name + ".datasets", BSON("00080018.Value" <<
                                                  UNIQUE_SOP_INSTANCE_UID_04));
    connection.remove(db_name + ".datasets", BSON("00080018.Value" <<
                                                  UNIQUE_SOP_INSTANCE_UID_05));
    connection.remove(db_name + ".datasets", BSON("00080018.Value" <<
                                                  UNIQUE_SOP_INSTANCE_UID_06));
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: stow_rs insert DICOM already register
 */
BOOST_FIXTURE_TEST_CASE(DicomAlreadyRegister, TestDataRequest)
{
    // Check SOP Instance UID not present in database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_08)), 0);

    std::string datasetfile(getenv("DOPAMINE_TEST_DICOMFILE_08"));

    std::stringstream dataset;
    dataset << content_type << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    // Open file
    std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
    BOOST_REQUIRE(file.is_open());
    // get length of file:
    int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));
    std::string output(length, '\0');
    // read data as a block:
    file.read (&output[0], output.size());
    // Close file
    file.close();

    dataset << output;
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    // First, insert into database
    dopamine::services::Stow_rs stowrs(path_info, "", dataset.str());
    BOOST_CHECK_NE(stowrs.get_response(), "");

    boost::property_tree::ptree ptree;
    std::stringstream xmlstream;
    xmlstream << stowrs.get_response();
    boost::property_tree::read_xml(xmlstream, ptree);
    BOOST_CHECK_EQUAL(ptree.find("NativeDicomModel") != ptree.not_found(), true);

    // check mandatory tag
    BOOST_CHECK(xmlstream.str().find("tag=\"00081199\"") != std::string::npos);

    // check tag error is missing
    BOOST_CHECK(xmlstream.str().find("tag=\"00081198\"") == std::string::npos);

    // check values
    BOOST_CHECK(xmlstream.str().find("1.2.840.10008.5.1.4.1.1.4") !=
                std::string::npos);
    BOOST_CHECK(xmlstream.str().find(UNIQUE_SOP_INSTANCE_UID_08) !=
                std::string::npos);

    // Check into database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_08)), 1);

    // Then try to register another time
    dopamine::services::Stow_rs stowrsagain(path_info, "", dataset.str());
    BOOST_CHECK_NE(stowrsagain.get_response(), "");

    boost::property_tree::ptree ptree2;
    std::stringstream xmlstream2;
    xmlstream2 << stowrsagain.get_response();
    boost::property_tree::read_xml(xmlstream2, ptree2);
    BOOST_CHECK_EQUAL(ptree2.find("NativeDicomModel") != ptree2.not_found(), true);

    // check mandatory tag
    BOOST_CHECK(xmlstream2.str().find("tag=\"00081199\"") != std::string::npos);

    // check tag error is missing
    BOOST_CHECK(xmlstream2.str().find("tag=\"00081198\"") == std::string::npos);

    // check values
    BOOST_CHECK(xmlstream2.str().find("1.2.840.10008.5.1.4.1.1.4") !=
                std::string::npos);
    BOOST_CHECK(xmlstream2.str().find(UNIQUE_SOP_INSTANCE_UID_08) !=
                std::string::npos);

    // Check into database (always present)
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_08)), 1);

    // remove dataset
    connection.remove(db_name + ".datasets", BSON("00080018.Value" <<
                                                  UNIQUE_SOP_INSTANCE_UID_08));
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: stow_rs check return status code
 */
BOOST_FIXTURE_TEST_CASE(ReturnStatusCode, TestDataRequest)
{
    // Check SOP Instance UID not present in database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_04)), 0);
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_05)), 0);
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_06)), 0);

    {
    std::string datasetfile(getenv("DOPAMINE_TEST_DICOMFILE_05"));

    std::stringstream dataset;
    dataset << content_type << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    // Open file
    std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
    BOOST_REQUIRE(file.is_open());
    // get length of file:
    int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));
    std::string output(length, '\0');
    // read data as a block:
    file.read (&output[0], output.size());
    // Close file
    file.close();

    dataset << output;
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    // First, insert into database
    dopamine::services::Stow_rs stowrs(path_info, "", dataset.str());
    BOOST_CHECK_NE(stowrs.get_response(), "");

    BOOST_CHECK_EQUAL(stowrs.get_status(), 200);
    BOOST_CHECK_EQUAL(stowrs.get_code(), "OK");

    // Check into database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_04)), 0);
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_05)), 1);
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_06)), 0);
    }

    {
    std::vector<std::string> files = { std::string(getenv("DOPAMINE_TEST_DICOMFILE_04")),
                                       std::string(getenv("DOPAMINE_TEST_DICOMFILE_05")),
                                       std::string(getenv("DOPAMINE_TEST_DICOMFILE_06")) };

    {
    std::stringstream dataset;
    dataset << content_type << "\n\n";
    int count = 0;
    for (std::string datasetfile : files)
    {
        dataset << "--" << boundary << "\n";
        dataset << dopamine::services::CONTENT_TYPE
                << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
        dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
                << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
        dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
                << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

        // Open file
        std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
        BOOST_REQUIRE(file.is_open());

        // get length of file:
        int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));

        std::string output(length, '\0');

        // read data as a block:
        file.read (&output[0], output.size());

        // Close file
        file.close();

        dataset << output;
        if (count == 1)
        {
            dataset << "error";
        }
        dataset << "\n" << "\n";
        ++count;
    }
    dataset << "--" << boundary << "--";

    dopamine::services::Stow_rs stowrs(path_info, "", dataset.str());
    BOOST_CHECK_NE(stowrs.get_response(), "");

    BOOST_CHECK_EQUAL(stowrs.get_status(), 202);
    BOOST_CHECK_EQUAL(stowrs.get_code(), "Accepted");
    }

    // Check into database
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_04)), 1);
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_05)), 1);
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_06)), 1);

    // All in error
    {
    std::stringstream dataset;
    dataset << content_type << "\n\n";
    for (std::string datasetfile : files)
    {
        dataset << "--" << boundary << "\n";
        dataset << dopamine::services::CONTENT_TYPE
                << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
        dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
                << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
        dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
                << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

        // Open file
        std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
        BOOST_REQUIRE(file.is_open());

        // get length of file:
        int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));

        std::string output(length, '\0');

        // read data as a block:
        file.read (&output[0], output.size());

        // Close file
        file.close();

        dataset << output;
        dataset << "error";

        dataset << "\n" << "\n";
    }
    dataset << "--" << boundary << "--";

    dopamine::services::Stow_rs stowrs(path_info, "", dataset.str());
    BOOST_CHECK_NE(stowrs.get_response(), "");

    BOOST_CHECK_EQUAL(stowrs.get_status(), 409);
    BOOST_CHECK_EQUAL(stowrs.get_code(), "Conflict");
    }

    // Check into database
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_04)), 1);
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_05)), 1);
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_06)), 1);
    }

    // remove dataset
    connection.remove(db_name + ".datasets", BSON("00080018.Value" <<
                                                  UNIQUE_SOP_INSTANCE_UID_04));
    connection.remove(db_name + ".datasets", BSON("00080018.Value" <<
                                                  UNIQUE_SOP_INSTANCE_UID_05));
    connection.remove(db_name + ".datasets", BSON("00080018.Value" <<
                                                  UNIQUE_SOP_INSTANCE_UID_06));
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: stow_rs insert 1 dataset for 1 identified Study
 */
BOOST_FIXTURE_TEST_CASE(InsertDatasetWithStudyInstanceUID, TestDataRequest)
{
    // Check SOP Instance UID not present in database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_07)), 0);

    std::string datasetfile(getenv("DOPAMINE_TEST_DICOMFILE_07"));

    std::stringstream dataset;
    dataset << content_type << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    // Open file
    std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
    BOOST_REQUIRE(file.is_open());
    // get length of file:
    int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));
    std::string output(length, '\0');
    // read data as a block:
    file.read (&output[0], output.size());
    // Close file
    file.close();

    dataset << output;
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    std::stringstream path_info_study;
    path_info_study << path_info << "/" << UNIQUE_STUDY_INSTANCE_UID;
    dopamine::services::Stow_rs stowrs(path_info_study.str(), "", dataset.str());

    BOOST_CHECK_NE(stowrs.get_response(), "");

    boost::property_tree::ptree ptree;
    std::stringstream xmlstream;
    xmlstream << stowrs.get_response();
    boost::property_tree::read_xml(xmlstream, ptree);
    BOOST_CHECK_EQUAL(ptree.find("NativeDicomModel") != ptree.not_found(), true);

    // check mandatory tag
    BOOST_CHECK(xmlstream.str().find("tag=\"00081190\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081199\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081150\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081155\"") != std::string::npos);

    // check tag error is missing
    BOOST_CHECK(xmlstream.str().find("tag=\"00081198\"") == std::string::npos);

    // check values
    BOOST_CHECK(xmlstream.str().find("1.2.840.10008.5.1.4.1.1.4") !=
                std::string::npos);
    BOOST_CHECK(xmlstream.str().find(UNIQUE_SOP_INSTANCE_UID_07) !=
                std::string::npos);

    // Check into database
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_07)), 1);

    // remove dataset
    connection.remove(db_name + ".datasets", BSON("00080018.Value" <<
                                                  UNIQUE_SOP_INSTANCE_UID_07));
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: stow_rs insert 1 dataset for 1 wrong Study
 */
BOOST_FIXTURE_TEST_CASE(InsertDatasetWithWrongStudyInstanceUID, TestDataRequest)
{
    // Check SOP Instance UID not present in database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_07)), 0);

    std::string datasetfile(getenv("DOPAMINE_TEST_DICOMFILE_07"));

    std::stringstream dataset;
    dataset << content_type << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    // Open file
    std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
    BOOST_REQUIRE(file.is_open());
    // get length of file:
    int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));
    std::string output(length, '\0');
    // read data as a block:
    file.read (&output[0], output.size());
    // Close file
    file.close();

    dataset << output;
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    std::stringstream path_info_study;
    path_info_study << path_info << "/" << "bad_value";
    dopamine::services::Stow_rs stowrs(path_info_study.str(), "", dataset.str());

    BOOST_CHECK_NE(stowrs.get_response(), "");

    boost::property_tree::ptree ptree;
    std::stringstream xmlstream;
    xmlstream << stowrs.get_response();
    boost::property_tree::read_xml(xmlstream, ptree);
    BOOST_CHECK_EQUAL(ptree.find("NativeDicomModel") != ptree.not_found(), true);

    // check mandatory tag
    BOOST_CHECK(xmlstream.str().find("tag=\"00081190\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081197\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081198\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081150\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081155\"") != std::string::npos);

    // check tag ReferencedSOPSequence is missing
    BOOST_CHECK(xmlstream.str().find("tag=\"00081199\"") == std::string::npos);

    // check values
    BOOST_CHECK(xmlstream.str().find("<Value number=\"1\">42752</Value>") !=
                std::string::npos); // error code A700
    BOOST_CHECK(xmlstream.str().find("1.2.840.10008.5.1.4.1.1.4") !=
                std::string::npos);
    BOOST_CHECK(xmlstream.str().find(UNIQUE_SOP_INSTANCE_UID_07) !=
                std::string::npos);

    // Check into database
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_07)), 0);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Bad content-type for the part
 */
BOOST_FIXTURE_TEST_CASE(BadPartContentType, TestDataRequest)
{
    // Check SOP Instance UID not present in database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_07)), 0);

    std::string datasetfile(getenv("DOPAMINE_TEST_DICOMFILE_07"));

    std::stringstream dataset;
    dataset << content_type << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOMXML << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    // Open file
    std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
    BOOST_REQUIRE(file.is_open());
    // get length of file:
    int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));
    std::string output(length, '\0');
    // read data as a block:
    file.read (&output[0], output.size());
    // Close file
    file.close();

    dataset << output;
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    dopamine::services::Stow_rs stowrs(path_info, "", dataset.str());
    BOOST_CHECK_NE(stowrs.get_response(), "");

    boost::property_tree::ptree ptree;
    std::stringstream xmlstream;
    xmlstream << stowrs.get_response();
    boost::property_tree::read_xml(xmlstream, ptree);
    BOOST_CHECK_EQUAL(ptree.find("NativeDicomModel") != ptree.not_found(), true);

    // check mandatory tag
    BOOST_CHECK(xmlstream.str().find("tag=\"00081190\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081197\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081198\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081150\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081155\"") != std::string::npos);

    // check tag ReferencedSOPSequence is missing
    BOOST_CHECK(xmlstream.str().find("tag=\"00081199\"") == std::string::npos);

    // check values
    BOOST_CHECK(xmlstream.str().find("<Value number=\"1\">272</Value>") !=
                std::string::npos); // error code 0110

    // Check into database
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_07)), 0);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Cannot read dataset
 */
BOOST_FIXTURE_TEST_CASE(UnableToReadDataset, TestDataRequest)
{
    // Check SOP Instance UID not present in database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_07)), 0);

    std::string datasetfile(getenv("DOPAMINE_TEST_DICOMFILE_07"));

    std::stringstream dataset;
    dataset << content_type << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    // Open file
    std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
    BOOST_REQUIRE(file.is_open());
    // get length of file:
    int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));
    std::string output(length, '\0');
    // read data as a block:
    file.read (&output[0], output.size());
    // Close file
    file.close();

    dataset << output << "BADEND";
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    dopamine::services::Stow_rs stowrs(path_info, "", dataset.str());

    BOOST_CHECK_NE(stowrs.get_response(), "");

    boost::property_tree::ptree ptree;
    std::stringstream xmlstream;
    xmlstream << stowrs.get_response();
    boost::property_tree::read_xml(xmlstream, ptree);
    BOOST_CHECK_EQUAL(ptree.find("NativeDicomModel") != ptree.not_found(), true);

    // check mandatory tag
    BOOST_CHECK(xmlstream.str().find("tag=\"00081190\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081197\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081198\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081150\"") != std::string::npos);
    BOOST_CHECK(xmlstream.str().find("tag=\"00081155\"") != std::string::npos);

    // check tag ReferencedSOPSequence is missing
    BOOST_CHECK(xmlstream.str().find("tag=\"00081199\"") == std::string::npos);

    // check values
    BOOST_CHECK(xmlstream.str().find("<Value number=\"1\">42752</Value>") !=
                std::string::npos); // error code A700

    // Check into database
    BOOST_CHECK_EQUAL(connection.count(db_name + ".datasets",
                                       BSON("00080018.Value" <<
                                            UNIQUE_SOP_INSTANCE_UID_07)), 0);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Content-type not supported
 */
BOOST_FIXTURE_TEST_CASE(TypeNotSupported, TestDataRequest)
{
    {
    std::stringstream dataset;
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_MULTIPART_RELATED << "; type="
            << dopamine::services::MIME_TYPE_APPLICATION_DICOMXML << "; "
            << dopamine::services::ATTRIBUT_BOUNDARY << boundary;
    dataset << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOMXML << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    dataset << "SOMETHING";
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    BOOST_CHECK_EXCEPTION(dopamine::services::Stow_rs(path_info, "", dataset.str()),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
                            { return (exc.status() == 415 &&
                                      exc.statusmessage() == "Unsupported Media Type"); });
    }

    {
    std::stringstream dataset;
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_MULTIPART_RELATED << "; type="
            << dopamine::services::MIME_TYPE_APPLICATION_JSON << "; "
            << dopamine::services::ATTRIBUT_BOUNDARY << boundary;
    dataset << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_JSON << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    dataset << "SOMETHING";
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    BOOST_CHECK_EXCEPTION(dopamine::services::Stow_rs(path_info, "", dataset.str()),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
                            { return (exc.status() == 415 &&
                                      exc.statusmessage() == "Unsupported Media Type"); });
    }
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Not allow
 */
BOOST_FIXTURE_TEST_CASE(NotAllowToStore, TestDataRequestNotAllow)
{
    // Check SOP Instance UID not present in database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_07)), 0);

    std::string datasetfile(getenv("DOPAMINE_TEST_DICOMFILE_07"));

    std::stringstream dataset;
    dataset << content_type << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    // Open file
    std::ifstream file(datasetfile, std::ifstream::binary | std::ifstream::in);
    BOOST_REQUIRE(file.is_open());
    // get length of file:
    int length = boost::filesystem::file_size(boost::filesystem::path(datasetfile));
    std::string output(length, '\0');
    // read data as a block:
    file.read (&output[0], output.size());
    // Close file
    file.close();

    dataset << output;
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    BOOST_CHECK_EXCEPTION(dopamine::services::Stow_rs(path_info, "", dataset.str(), "ME"),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
                            { return (exc.status() == 401 &&
                                      exc.statusmessage() == "Unauthorized"); });

    // Check SOP Instance UID not present in database
    BOOST_REQUIRE_EQUAL(connection.count(db_name + ".datasets",
                                         BSON("00080018.Value" <<
                                              UNIQUE_SOP_INSTANCE_UID_07)), 0);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Bad parameter (first parameter should be studies)
 */
BOOST_FIXTURE_TEST_CASE(BadParameter, TestDataRequest)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Stow_rs("/badValue", "", content_type),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
                            { return (exc.status() == 400 &&
                                      exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Missing parameter
 */
BOOST_FIXTURE_TEST_CASE(MissingParameter, TestDataRequest)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Stow_rs("", "", content_type),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
                            { return (exc.status() == 400 &&
                                      exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Too Many parameter
 */
BOOST_FIXTURE_TEST_CASE(TooManyParameter, TestDataRequest)
{
    BOOST_CHECK_EXCEPTION(dopamine::services::Stow_rs("/studies/1.2.3/tooMany", "", content_type),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
                            { return (exc.status() == 400 &&
                                      exc.statusmessage() == "Bad Request"); });
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Bad Content-Type
 */
BOOST_FIXTURE_TEST_CASE(BadContentType, TestDataRequest)
{
    {
    std::stringstream dataset;
    dataset << dopamine::services::CONTENT_TYPE << "; "
            << dopamine::services::ATTRIBUT_BOUNDARY << boundary;
    dataset << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOMXML << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    dataset << "SOMETHING";
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    BOOST_CHECK_EXCEPTION(dopamine::services::Stow_rs(path_info, "", dataset.str()),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
                            { return (exc.status() == 400 &&
                                      exc.statusmessage() == "Bad Request"); });
    }

    {
    std::stringstream dataset;
    dataset << dopamine::services::CONTENT_TYPE
            << "BADVALUE" << "; type="
            << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "; "
            << dopamine::services::ATTRIBUT_BOUNDARY << boundary;
    dataset << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_APPLICATION_DICOM << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    dataset << "SOMETHING";
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    BOOST_CHECK_EXCEPTION(dopamine::services::Stow_rs(path_info, "", dataset.str()),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
                            { return (exc.status() == 400 &&
                                      exc.statusmessage() == "Bad Request"); });
    }
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Media type not supported
 */
BOOST_FIXTURE_TEST_CASE(UnknownMediaType, TestDataRequest)
{
    std::stringstream dataset;
    dataset << dopamine::services::CONTENT_TYPE
            << dopamine::services::MIME_TYPE_MULTIPART_RELATED << "; type="
            << "application/text" << "; "
            << dopamine::services::ATTRIBUT_BOUNDARY << boundary;
    dataset << "\n\n";
    dataset << "--" << boundary << "\n";
    dataset << dopamine::services::CONTENT_TYPE
            << "application/text" << "\n";
    dataset << dopamine::services::CONTENT_DISPOSITION_ATTACHMENT << " "
            << dopamine::services::ATTRIBUT_FILENAME << "myfile" << "\n";
    dataset << dopamine::services::CONTENT_TRANSFER_ENCODING
            << dopamine::services::TRANSFER_ENCODING_BINARY << "\n" << "\n";

    dataset << "SOMETHING";
    dataset << "\n" << "\n";
    dataset << "--" << boundary << "--";

    BOOST_CHECK_EXCEPTION(dopamine::services::Stow_rs(path_info, "", dataset.str()),
                          dopamine::services::WebServiceException,
                          [] (dopamine::services::WebServiceException const exc)
                            { return (exc.status() == 400 &&
                                      exc.statusmessage() == "Bad Request"); });
}
