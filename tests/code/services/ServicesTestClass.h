/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _90904ff2_c253_4e33_86ce_7bad503a746c
#define _90904ff2_c253_4e33_86ce_7bad503a746c

#include <stdlib.h>
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <mongo/client/dbclient.h>

#include "ConverterBSON/Dataset/DataSetToBSON.h"
#include "ConverterBSON/Dataset/IsPrivateTag.h"
#include "ConverterBSON/Dataset/VRMatch.h"
#include "core/ConfigurationPACS.h"
#include "services/ServicesTools.h"

#include <dcmtk/dcmdata/dcistrmb.h>
#include <dcmtk/dcmdata/dcwcache.h>

std::string const STUDY_INSTANCE_UID_01_01 = "2.16.756.5.5.100.3611280983.19057.1364461809.7789";
std::string const STUDY_INSTANCE_UID_02_01 = "2.16.756.5.5.100.1333920868.19866.1424334602.23";
std::string const STUDY_INSTANCE_UID_03_01 = "2.16.756.5.5.100.3611280983.19057.9964462499.7789";
std::string const STUDY_INSTANCE_UID_BIG = "2.16.756.5.5.100.12345.54321";
std::string const STUDY_INSTANCE_UID_BIG_02 = "2.16.756.5.5.100.12346.64321";

std::string const SERIES_INSTANCE_UID_01_01_01 = "2.16.756.5.5.100.3611280983.20092.1364462458.1";
std::string const SERIES_INSTANCE_UID_03_01_01 = "2.16.756.5.5.100.3611280983.20092.9964462499.1";
std::string const SERIES_INSTANCE_UID_BIG = "2.16.756.5.5.100.12346.64321.1";
std::string const SERIES_INSTANCE_UID_BIG_02 = "2.16.756.5.5.100.12346.64321.1";

std::string const SOP_INSTANCE_UID_01_01_01_01 = "2.16.756.5.5.100.3611280983.20092.1364462458.1.0";
std::string const SOP_INSTANCE_UID_03_01_01_01 = "2.16.756.5.5.100.3611280983.20092.9964462499.1.0";
std::string const SOP_INSTANCE_UID_03_02_01_01 = "2.16.756.5.5.100.3611280983.20092.9964462499.1.9";
std::string const SOP_INSTANCE_UID_04_01_01_01 = "1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489112";
std::string const SOP_INSTANCE_UID_04_01_01_02 = "1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489113";
std::string const SOP_INSTANCE_UID_04_01_01_03 = "1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489114";
std::string const SOP_INSTANCE_UID_BIG_01 = "2.16.756.5.5.100.12345.54321.1.0";
std::string const SOP_INSTANCE_UID_BIG_02 = "2.16.756.5.5.100.12346.64321.1.0";

/**
 * Test fixture getting the environment variables required to test the services
 */
class ServicesTestClass
{
public:

    mongo::DBClientConnection connection;
    std::string db_name;
    std::vector<std::string> SOPInstanceUIDs;
    std::vector<std::string> SOPInstanceUIDs_GridFS;

    ServicesTestClass()
    {
        // Load configuration
        dopamine::ConfigurationPACS::
                get_instance().parse(_get_env_variable("DOPAMINE_TEST_CONFIG"));

        // Create DataBase Connection
        dopamine::services::create_db_connection(connection, db_name);

        // Add data into DataBase
        this->_insert_data();
        this->_insert_big_dataset();
    }

    virtual ~ServicesTestClass()
    {
        // Remove data from DataBase
        this->_remove_data();

        this->_remove_constraints();

        this->_reset_authorization();

        // Delete configuration
        dopamine::ConfigurationPACS::delete_instance();
        sleep(1);
    }

protected:
    void set_authorization(std::string const & service, std::string const & user,
                           mongo::BSONObj const & constraint = mongo::BSONObj())
    {
        mongo::BSONObj value = BSON("principal_name" << user <<
                                    "principal_type" << "" <<
                                    "service" << service <<
                                    "dataset" << constraint);
        this->connection.update(this->db_name + ".authorization",
                                BSON("service" << service), value);
        std::string result = this->connection.getLastError(this->db_name);
        if (result != "") // empty string if no error
        {
            BOOST_FAIL(result);
        }

        _services.push_back(service);
    }

    void add_constraint(std::string const & service,
                        std::string const & user,
                        mongo::BSONObj const & constraint)
    {
        mongo::BSONObj value = BSON("principal_name" << user <<
                                    "principal_type" << "" <<
                                    "service" << service <<
                                    "dataset" << constraint);
        this->connection.insert(this->db_name + ".authorization", value);
        std::string result = this->connection.getLastError(this->db_name);
        if (result != "") // empty string if no error
        {
            BOOST_FAIL(result);
        }

        this->_constraints.push_back(value);
    }

private:
    std::vector<mongo::BSONObj> _constraints;
    std::vector<std::string> _services;

    std::string _get_env_variable(std::string const & name) const
    {
        char* value = getenv(name.c_str());
        if(value == NULL)
        {
            BOOST_FAIL(name + " is not defined");
        }
        return value;
    }

    void _insert_data()
    {
        std::vector<std::string> testfiles = { "DOPAMINE_TEST_DICOMFILE_01",
                                               "DOPAMINE_TEST_DICOMFILE_02",
                                               "DOPAMINE_TEST_DICOMFILE_03",
                                               "DOPAMINE_TEST_DICOMFILE_04"
                                               //"DOPAMINE_TEST_DICOMFILE_05", // test storage
                                               //"DOPAMINE_TEST_DICOMFILE_06", // test storage
                                               //"DOPAMINE_TEST_DICOMFILE_07", // test storage
                                               //"DOPAMINE_TEST_DICOMFILE_08", // test storage
                                               //"DOPAMINE_TEST_DICOMFILE_09", // test storage
                                             };

        // Remove files store by services
        this->SOPInstanceUIDs.push_back(SOP_INSTANCE_UID_03_01_01_01);
        this->SOPInstanceUIDs.push_back(SOP_INSTANCE_UID_03_02_01_01);
        this->SOPInstanceUIDs.push_back(SOP_INSTANCE_UID_04_01_01_01);
        this->SOPInstanceUIDs.push_back(SOP_INSTANCE_UID_04_01_01_02);
        this->SOPInstanceUIDs.push_back(SOP_INSTANCE_UID_04_01_01_03);

        std::stringstream streamTable;
        streamTable << this->db_name << ".datasets";
        for (std::string testfile : testfiles)
        {
            // Get file name
            std::string filename = this->_get_env_variable(testfile);

            // Load Dataset
            DcmFileFormat fileformat;
            OFCondition condition = fileformat.loadFile(filename.c_str());
            if (condition.bad())
            {
                BOOST_FAIL(condition.text());
            }

            // Keep the original transfer syntax (if available)
            E_TransferSyntax xfer = fileformat.getMetaInfo()->getOriginalXfer();
            if (xfer == EXS_Unknown)
            {
              // No information about the original transfer syntax: This is
              // most probably a DICOM dataset that was read from memory.
              xfer = EXS_LittleEndianExplicit;
            }
            fileformat.validateMetaInfo(xfer);
            fileformat.removeInvalidGroups();

            // Create a memory buffer with the proper size
            uint32_t size = fileformat.calcElementLength(xfer, EET_ExplicitLength);
            std::string buffer;
            buffer.resize(size);

            // Create buffer for DCMTK
            DcmOutputBufferStream* outputstream = new DcmOutputBufferStream(&buffer[0], size);

            // Fill the memory buffer with the meta-header and the dataset
            fileformat.transferInit();
            condition = fileformat.write(*outputstream,
                                         xfer,
                                         EET_ExplicitLength, NULL);
            fileformat.transferEnd();

            delete outputstream;

            if (condition.bad())
            {
                BOOST_FAIL(condition.text());
            }

            DcmDataset* dataset = fileformat.getDataset();

            // Convert Dataset into BSON object
            dopamine::converterBSON::DataSetToBSON dataset_to_bson;
            dataset_to_bson.get_filters().push_back(std::make_pair(
                dopamine::converterBSON::IsPrivateTag::New(),
                dopamine::converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
            dataset_to_bson.get_filters().push_back(std::make_pair(
                dopamine::converterBSON::VRMatch::New(EVR_OB),
                dopamine::converterBSON:: DataSetToBSON::FilterAction::EXCLUDE));
            dataset_to_bson.get_filters().push_back(std::make_pair(
                dopamine::converterBSON::VRMatch::New(EVR_OF),
                dopamine::converterBSON:: DataSetToBSON::FilterAction::EXCLUDE));
            dataset_to_bson.get_filters().push_back(std::make_pair(
                dopamine::converterBSON::VRMatch::New(EVR_OW),
                dopamine::converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
            dataset_to_bson.get_filters().push_back(std::make_pair(
                dopamine::converterBSON::VRMatch::New(EVR_UN),
                dopamine::converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
            dataset_to_bson.set_default_filter(dopamine::converterBSON::DataSetToBSON::FilterAction::INCLUDE);
            mongo::BSONObj object = dataset_to_bson.from_dataset(dataset);
            if (!object.isValid() || object.isEmpty())
            {
                BOOST_FAIL("Could not convert Dataset to BSON");
            }

            // Get the SOPInstanceUID for delete
            this->SOPInstanceUIDs.push_back(object.getField("00080018").Obj().getField("Value").Array()[0].String());

            // Create BSON to insert into DataBase
            mongo::BSONObjBuilder builder;
            builder.appendElements(object);
            builder.appendBinData("Content", buffer.size(),
                                  mongo::BinDataGeneral, buffer.c_str());

            // insert into DataBase
            this->connection.insert(streamTable.str(), builder.obj());
            std::string result = this->connection.getLastError(this->db_name);
            if (result != "") // empty string if no error
            {
                BOOST_FAIL(result);
            }
        }

        // insert entry with bad Content
        mongo::BSONObjBuilder badbuilder;
        badbuilder << "00080018" << BSON("vr" << "UI" << "Value" << BSON_ARRAY("2.16.756.5.5.100.3611280983.20092.123456789.0.0"));
        badbuilder << "0020000d" << BSON("vr" << "UI" << "Value" << BSON_ARRAY("2.16.756.5.5.100.3611280983.20092.123456789"));
        badbuilder << "0020000e" << BSON("vr" << "UI" << "Value" << BSON_ARRAY("2.16.756.5.5.100.3611280983.20092.123456789.0"));
        badbuilder.appendNumber("Content", 1);

        this->SOPInstanceUIDs.push_back("2.16.756.5.5.100.3611280983.20092.123456789.0.0");

        this->connection.insert(streamTable.str(), badbuilder.obj());
        std::string result = this->connection.getLastError(this->db_name);
        if (result != "") // empty string if no error
        {
            BOOST_FAIL(result);
        }
    }

    void _insert_big_dataset()
    {
        OFCondition condition = EC_Normal;

        // Create the dataset
        DcmDataset * dataset = new DcmDataset();
        condition = dataset->putAndInsertOFStringArray(DCM_SOPInstanceUID,
                                                       OFString(SOP_INSTANCE_UID_BIG_01.c_str()));
        BOOST_REQUIRE(condition.good());
        condition = dataset->putAndInsertOFStringArray(DCM_StudyInstanceUID,
                                                       OFString(STUDY_INSTANCE_UID_BIG.c_str()));
        BOOST_REQUIRE(condition.good());
        condition = dataset->putAndInsertOFStringArray(DCM_SeriesInstanceUID,
                                                       OFString(SERIES_INSTANCE_UID_BIG.c_str()));
        BOOST_REQUIRE(condition.good());
        condition = dataset->putAndInsertOFStringArray(DCM_PatientName,
                                                       OFString("Big^Data"));
        BOOST_REQUIRE(condition.good());
        condition = dataset->putAndInsertOFStringArray(DCM_Modality, OFString("MR"));
        BOOST_REQUIRE(condition.good());
        condition = dataset->putAndInsertOFStringArray(DCM_SOPClassUID, OFString(UID_MRImageStorage));
        BOOST_REQUIRE(condition.good());
        condition = dataset->putAndInsertOFStringArray(DCM_PatientID, "123");
        BOOST_REQUIRE(condition.good());
        // Binary
        size_t vectorsize = 4096*4096;
        std::vector<Uint8> value(vectorsize, 0);
        condition = dataset->putAndInsertUint8Array(DCM_PixelData, &value[0], vectorsize);
        BOOST_REQUIRE(condition.good());

        // Create Dataset with Header
        DcmFileFormat fileformat(dataset);

        // Keep the original transfer syntax (if available)
        E_TransferSyntax xfer = fileformat.getMetaInfo()->getOriginalXfer();
        if (xfer == EXS_Unknown)
        {
          // No information about the original transfer syntax: This is
          // most probably a DICOM dataset that was read from memory.
          xfer = EXS_LittleEndianExplicit;
        }
        fileformat.validateMetaInfo(xfer);
        fileformat.removeInvalidGroups();

        // Create a memory buffer with the proper size
        uint32_t size = fileformat.calcElementLength(xfer, EET_ExplicitLength);
        std::string buffer;
        buffer.resize(size);

        // Create buffer for DCMTK
        DcmOutputBufferStream* outputstream = new DcmOutputBufferStream(&buffer[0], size);

        // Fill the memory buffer with the meta-header and the dataset
        fileformat.transferInit();
        condition = fileformat.write(*outputstream,
                                     xfer,
                                     EET_ExplicitLength, NULL);
        fileformat.transferEnd();

        delete outputstream;

        if (condition.bad())
        {
            BOOST_FAIL(condition.text());
        }

        // Convert Dataset into BSON object
        dopamine::converterBSON::DataSetToBSON dataset_to_bson;
        dataset_to_bson.get_filters().push_back(std::make_pair(
            dopamine::converterBSON::IsPrivateTag::New(),
            dopamine::converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            dopamine::converterBSON::VRMatch::New(EVR_OB),
            dopamine::converterBSON:: DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            dopamine::converterBSON::VRMatch::New(EVR_OF),
            dopamine::converterBSON:: DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            dopamine::converterBSON::VRMatch::New(EVR_OW),
            dopamine::converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            dopamine::converterBSON::VRMatch::New(EVR_UN),
            dopamine::converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.set_default_filter(dopamine::converterBSON::DataSetToBSON::FilterAction::INCLUDE);
        mongo::BSONObj object = dataset_to_bson.from_dataset(dataset);
        if (!object.isValid() || object.isEmpty())
        {
            BOOST_FAIL("Could not convert Dataset to BSON");
        }

        // Get the SOPInstanceUID for delete
        this->SOPInstanceUIDs.push_back(SOP_INSTANCE_UID_BIG_01);
        this->SOPInstanceUIDs_GridFS.push_back(SOP_INSTANCE_UID_BIG_01);
        // SOPInstanceUID used by Stow
        this->SOPInstanceUIDs.push_back(SOP_INSTANCE_UID_BIG_02);
        this->SOPInstanceUIDs_GridFS.push_back(SOP_INSTANCE_UID_BIG_02);

        // insert into GridSF
        mongo::GridFS gridfs(connection, db_name);
        mongo::BSONObj objret =
                gridfs.storeFile(buffer.c_str(),
                                 buffer.size(),
                                 SOP_INSTANCE_UID_BIG_01);

        if (!objret.isValid() || objret.isEmpty())
        {
            BOOST_FAIL("Cannot store into GridFS");
        }

        // Create BSON to insert into DataBase
        mongo::BSONObjBuilder builder;
        builder.appendElements(object);
        builder << "Content" << objret.getField("_id").OID().toString();

        std::stringstream streamTable;
        streamTable << this->db_name << ".datasets";
        // insert into DataBase
        this->connection.insert(streamTable.str(), builder.obj());
        std::string result = this->connection.getLastError(this->db_name);
        if (result != "") // empty string if no error
        {
            BOOST_FAIL(result);
        }

        delete dataset;
    }

    void _remove_data()
    {
        // Delete all data
        for (std::string const SOPInstanceUID : this->SOPInstanceUIDs)
        {
            this->connection.remove(this->db_name + ".datasets",
                                    BSON("00080018.Value" << SOPInstanceUID));
        }

        // Delete data from GridFS
        mongo::GridFS gridfs(connection, db_name);
        for (std::string const SOPInstanceUIDgridfs : this->SOPInstanceUIDs_GridFS)
        {
            gridfs.removeFile(SOPInstanceUIDgridfs);
        }
    }

    void _remove_constraints()
    {
        for (auto const constraint : this->_constraints)
        {
            this->connection.remove(this->db_name + ".authorization", constraint);
        }
    }

    void _reset_authorization()
    {
        for (auto const service : this->_services)
        {
            mongo::BSONObj value = BSON("principal_name" << "" <<
                                        "principal_type" << "" <<
                                        "service" << service <<
                                        "dataset" << mongo::BSONObj());
            this->connection.update(this->db_name + ".authorization",
                                    BSON("service" << service), value);
            std::string result = this->connection.getLastError(this->db_name);
            if (result != "") // empty string if no error
            {
                BOOST_FAIL(result);
            }
        }
    }

};

#endif // _90904ff2_c253_4e33_86ce_7bad503a746c
