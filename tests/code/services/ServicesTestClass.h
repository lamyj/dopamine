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

#include <dcmtkpp/DataSet.h>
#include <dcmtkpp/Reader.h>
#include <dcmtkpp/Writer.h>

#include <mongo/client/dbclient.h>

#include "ConverterBSON/bson_converter.h"
#include "ConverterBSON/IsPrivateTag.h"
#include "ConverterBSON/VRMatch.h"
#include "core/ConfigurationPACS.h"
#include "services/ServicesTools.h"

//#include <dcmtk/dcmdata/dcistrmb.h>
//#include <dcmtk/dcmdata/dcwcache.h>

std::string const STUDY_INSTANCE_UID_01_01 =
        "2.16.756.5.5.100.3611280983.19057.1364461809.7789";
std::string const STUDY_INSTANCE_UID_02_01 =
        "2.16.756.5.5.100.1333920868.19866.1424334602.23";
std::string const STUDY_INSTANCE_UID_03_01 =
        "2.16.756.5.5.100.3611280983.19057.9964462499.7789";
std::string const STUDY_INSTANCE_UID_BIG = "2.16.756.5.5.100.12345.54321";
std::string const STUDY_INSTANCE_UID_BIG_02 = "2.16.756.5.5.100.12346.64321";
std::string const STUDY_INSTANCE_UID_BAD =
        "2.16.756.5.5.100.3611280983.20092.123456789";

std::string const SERIES_INSTANCE_UID_01_01_01 =
        "2.16.756.5.5.100.3611280983.20092.1364462458.1";
std::string const SERIES_INSTANCE_UID_03_01_01 =
        "2.16.756.5.5.100.3611280983.20092.9964462499.1";
std::string const SERIES_INSTANCE_UID_BIG = "2.16.756.5.5.100.12346.64321.1";
std::string const SERIES_INSTANCE_UID_BIG_02 =
        "2.16.756.5.5.100.12346.64321.1";
std::string const SERIES_INSTANCE_UID_BAD =
        "2.16.756.5.5.100.3611280983.20092.123456789.0";

std::string const SOP_INSTANCE_UID_01_01_01_01 =
        "2.16.756.5.5.100.3611280983.20092.1364462458.1.0";
std::string const SOP_INSTANCE_UID_03_01_01_01 =
        "2.16.756.5.5.100.3611280983.20092.9964462499.1.0";
std::string const SOP_INSTANCE_UID_03_02_01_01 =
        "2.16.756.5.5.100.3611280983.20092.9964462499.1.9";
std::string const SOP_INSTANCE_UID_04_01_01_01 =
        "1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489112";
std::string const SOP_INSTANCE_UID_04_01_01_02 =
        "1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489113";
std::string const SOP_INSTANCE_UID_04_01_01_03 =
        "1.2.276.0.7230010.3.1.4.8323329.7922.1432822445.489114";
std::string const SOP_INSTANCE_UID_BIG_01 =
        "2.16.756.5.5.100.12345.54321.1.0";
std::string const SOP_INSTANCE_UID_BIG_02 =
        "2.16.756.5.5.100.12346.64321.1.0";
std::string const SOP_INSTANCE_UID_BAD =
        "2.16.756.5.5.100.3611280983.20092.123456789.0.0";

/**
 * \class Test fixture getting the environment variables
 *        required to test the services
 */
class ServicesTestClass
{
public:

    mongo::DBClientConnection connection;
    std::string db_name;

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
    std::vector<std::string> _sop_instance_uids;
    std::vector<std::string> _sop_instance_uids_gridfs;

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
                                             };

        // Remove files store by services
        this->_sop_instance_uids.push_back(SOP_INSTANCE_UID_03_01_01_01);
        this->_sop_instance_uids.push_back(SOP_INSTANCE_UID_03_02_01_01);
        this->_sop_instance_uids.push_back(SOP_INSTANCE_UID_04_01_01_01);
        this->_sop_instance_uids.push_back(SOP_INSTANCE_UID_04_01_01_02);
        this->_sop_instance_uids.push_back(SOP_INSTANCE_UID_04_01_01_03);

        std::stringstream streamTable;
        streamTable << this->db_name << ".datasets";
        for (std::string testfile : testfiles)
        {
            // Get file name
            std::string const filename = this->_get_env_variable(testfile);

            std::ifstream stream(filename, std::ios::in | std::ios::binary);

            std::pair<dcmtkpp::DataSet, dcmtkpp::DataSet> file;
            try
            {
                file = dcmtkpp::Reader::read_file(stream);
            }
            catch(std::exception & e)
            {
                std::stringstream error;
                error << "Could not read " << filename << ": " << e.what();
                BOOST_FAIL(error.str());
            }

            auto const & data_set = file.second;

            // Convert Dataset into BSON object
            dopamine::Filters filters = {};
            filters.push_back(std::make_pair(
                dopamine::converterBSON::IsPrivateTag::New(),
                dopamine::FilterAction::EXCLUDE));
            filters.push_back(std::make_pair(
                dopamine::converterBSON::VRMatch::New(dcmtkpp::VR::OB),
                dopamine::FilterAction::EXCLUDE));
            filters.push_back(std::make_pair(
                dopamine::converterBSON::VRMatch::New(dcmtkpp::VR::OF),
                dopamine::FilterAction::EXCLUDE));
            filters.push_back(std::make_pair(
                dopamine::converterBSON::VRMatch::New(dcmtkpp::VR::OW),
                dopamine::FilterAction::EXCLUDE));
            filters.push_back(std::make_pair(
                dopamine::converterBSON::VRMatch::New(dcmtkpp::VR::UN),
                dopamine::FilterAction::EXCLUDE));

            mongo::BSONObj object =
                    dopamine::as_bson(data_set, dopamine::FilterAction::INCLUDE,
                                      filters);
            if (!object.isValid() || object.isEmpty())
            {
                BOOST_FAIL("Could not convert Dataset to BSON");
            }

            // Get the SOPInstanceUID for delete
            this->_sop_instance_uids.push_back(
                        object["00080018"].Obj()["Value"].Array()[0].String());

            std::stringstream stream_dataset;
            dcmtkpp::Writer::write_file(data_set, stream_dataset);

            // Create a memory buffer with the proper size
            std::string const buffer = stream_dataset.str();

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
        badbuilder << "00080018"
                   << BSON("vr" << "UI" << "Value" <<
                           BSON_ARRAY(SOP_INSTANCE_UID_BAD));
        badbuilder << "0020000d"
                   << BSON("vr" << "UI" << "Value" <<
                           BSON_ARRAY(STUDY_INSTANCE_UID_BAD));
        badbuilder << "0020000e"
                   << BSON("vr" << "UI" << "Value" <<
                           BSON_ARRAY(SERIES_INSTANCE_UID_BAD));
        badbuilder.appendNumber("Content", 1);

        this->_sop_instance_uids.push_back(SOP_INSTANCE_UID_BAD);

        this->connection.insert(streamTable.str(), badbuilder.obj());
        std::string result = this->connection.getLastError(this->db_name);
        if (result != "") // empty string if no error
        {
            BOOST_FAIL(result);
        }
    }

    void _insert_big_dataset()
    {
        // Create the dataset
        dcmtkpp::DataSet dataset;

        dataset.add(dcmtkpp::registry::SOPInstanceUID, dcmtkpp::Element({SOP_INSTANCE_UID_BIG_01}, dcmtkpp::VR::UI));
        dataset.add(dcmtkpp::registry::StudyInstanceUID, dcmtkpp::Element({STUDY_INSTANCE_UID_BIG}, dcmtkpp::VR::UI));
        dataset.add(dcmtkpp::registry::SeriesInstanceUID, dcmtkpp::Element({SERIES_INSTANCE_UID_BIG}, dcmtkpp::VR::UI));
        dataset.add(dcmtkpp::registry::PatientName, dcmtkpp::Element({"Big^Data"}, dcmtkpp::VR::PN));
        dataset.add(dcmtkpp::registry::Modality, dcmtkpp::Element({"MR"}, dcmtkpp::VR::CS));
        dataset.add(dcmtkpp::registry::SOPClassUID, dcmtkpp::Element({dcmtkpp::registry::MRImageStorage}, dcmtkpp::VR::UI));
        dataset.add(dcmtkpp::registry::PatientID, dcmtkpp::Element({"123"}, dcmtkpp::VR::LO));

        // Binary
        size_t vectorsize = 4096*4096;
        dcmtkpp::Value::Binary value(vectorsize, 0);
        dataset.add(dcmtkpp::registry::PixelData, dcmtkpp::Element(value, dcmtkpp::VR::OW));

        // Convert Dataset into BSON object
        dopamine::Filters filters = {};
        filters.push_back(std::make_pair(
            dopamine::converterBSON::IsPrivateTag::New(),
            dopamine::FilterAction::EXCLUDE));
        filters.push_back(std::make_pair(
            dopamine::converterBSON::VRMatch::New(dcmtkpp::VR::OB),
            dopamine::FilterAction::EXCLUDE));
        filters.push_back(std::make_pair(
            dopamine::converterBSON::VRMatch::New(dcmtkpp::VR::OF),
            dopamine::FilterAction::EXCLUDE));
        filters.push_back(std::make_pair(
            dopamine::converterBSON::VRMatch::New(dcmtkpp::VR::OW),
            dopamine::FilterAction::EXCLUDE));
        filters.push_back(std::make_pair(
            dopamine::converterBSON::VRMatch::New(dcmtkpp::VR::UN),
            dopamine::FilterAction::EXCLUDE));

        mongo::BSONObj const object =
                dopamine::as_bson(dataset, dopamine::FilterAction::INCLUDE,
                                  filters);
        if (!object.isValid() || object.isEmpty())
        {
            BOOST_FAIL("Could not convert Dataset to BSON");
        }

        // Get the SOPInstanceUID for delete
        this->_sop_instance_uids.push_back(SOP_INSTANCE_UID_BIG_01);
        this->_sop_instance_uids_gridfs.push_back(SOP_INSTANCE_UID_BIG_01);
        // SOPInstanceUID used by Stow
        this->_sop_instance_uids.push_back(SOP_INSTANCE_UID_BIG_02);
        this->_sop_instance_uids_gridfs.push_back(SOP_INSTANCE_UID_BIG_02);

        std::stringstream stream_dataset;
        dcmtkpp::Writer::write_file(dataset, stream_dataset);
        std::string const buffer = stream_dataset.str();

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
    }

    void _remove_data()
    {
        // Delete all data
        for (std::string const SOPInstanceUID : this->_sop_instance_uids)
        {
            this->connection.remove(this->db_name + ".datasets",
                                    BSON("00080018.Value" << SOPInstanceUID));
        }

        // Delete data from GridFS
        mongo::GridFS gridfs(connection, db_name);
        for (std::string const SOPInstanceUIDgridfs :
             this->_sop_instance_uids_gridfs)
        {
            gridfs.removeFile(SOPInstanceUIDgridfs);
        }
    }

    void _remove_constraints()
    {
        for (auto const constraint : this->_constraints)
        {
            this->connection.remove(this->db_name + ".authorization",
                                    constraint);
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
