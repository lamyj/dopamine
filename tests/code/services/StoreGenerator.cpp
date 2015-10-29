/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleQueryRetrieveGenerator
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/DataSet.h>
#include <dcmtkpp/Response.h>

#include "core/ExceptionPACS.h"
#include "ServicesTestClass.h"
#include "services/StoreGenerator.h"
#include "services/ServicesTools.h"

class TestDataGenerator_constraint : public ServicesTestClass
{
public:
    TestDataGenerator_constraint() : ServicesTestClass()
    {
        mongo::BSONObjBuilder builder;
        builder << "00080060" << "MR";
        this->add_constraint(dopamine::services::Service_Store,
                             "root", builder.obj());
    }

    virtual ~TestDataGenerator_constraint()
    {
        // Nothing to do
    }
};

class TestDataGenerator_badconnection
{
public:
    TestDataGenerator_badconnection()
    {
        std::string NetworkConfFILE(getenv("DOPAMINE_TEST_BADCONFIG"));
        dopamine::ConfigurationPACS::get_instance().parse(NetworkConfFILE);
    }

    ~TestDataGenerator_badconnection()
    {
        dopamine::ConfigurationPACS::delete_instance();
        sleep(1);
    }
};

class TestDataGenerator_notallow : public ServicesTestClass
{
public:
    TestDataGenerator_notallow() : ServicesTestClass()
    {
        mongo::BSONObjBuilder builder;
        builder.appendRegex("00080018", "Unknown");
        this->set_authorization(dopamine::services::Service_Store,
                                "root", builder.obj());

        mongo::BSONObjBuilder builder2;
        builder2 << "00080060" << "NotMR";
        this->add_constraint(dopamine::services::Service_Store,
                             "not_me", builder2.obj());
    }

    virtual ~TestDataGenerator_notallow()
    {
        // Nothing to do
    }
};

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_FIXTURE_TEST_CASE(Constructor, ServicesTestClass)
{
    dopamine::services::StoreGenerator * storegenerator =
            new dopamine::services::StoreGenerator("");

    BOOST_CHECK_EQUAL(storegenerator != NULL, true);

    delete storegenerator;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Getter and Setter
 */
BOOST_FIXTURE_TEST_CASE(Accessors, ServicesTestClass)
{
    dopamine::services::StoreGenerator generator("");

    // Default initialization
    BOOST_CHECK_EQUAL(generator.get_calling_aptitle(), "");
    BOOST_CHECK(generator.get_dataset().empty());
    BOOST_CHECK_EQUAL(generator.is_allow(), false);

    // Setter
    generator.set_calling_aptitle("LOCAL");
    BOOST_CHECK_EQUAL(generator.get_calling_aptitle(), "LOCAL");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Function Cancel
 */
BOOST_FIXTURE_TEST_CASE(Cancel, ServicesTestClass)
{
    dopamine::services::StoreGenerator generator("");

    // Not yet implemented
    generator.cancel();
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Empty request
 */
BOOST_FIXTURE_TEST_CASE(Empty_Request, ServicesTestClass)
{
    mongo::unique_ptr<mongo::DBClientCursor> cursor =
            connection.query(db_name + ".datasets",
                             BSON("00080018.Value" <<
                                  SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dopamine::services::StoreGenerator generator("");

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPClassUID, dcmtkpp::Element({dcmtkpp::registry::MRImageStorage}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SOPInstanceUID, dcmtkpp::Element({SOP_INSTANCE_UID_03_01_01_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::StudyInstanceUID, dcmtkpp::Element({STUDY_INSTANCE_UID_03_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SeriesInstanceUID, dcmtkpp::Element({SERIES_INSTANCE_UID_03_01_01}, dcmtkpp::VR::UI));

    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, dcmtkpp::Response::Pending);

    cursor = connection.query(db_name + ".datasets",
                              BSON("00080018.Value" <<
                                   SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK_EQUAL(response.hasField("00080018"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000d"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000e"), true);

    BOOST_CHECK_EQUAL(cursor->more(), false);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Insert all attribute VR
 */
BOOST_FIXTURE_TEST_CASE(Insert_All_VR, ServicesTestClass)
{
    mongo::unique_ptr<mongo::DBClientCursor> cursor =
            connection.query(db_name + ".datasets",
                             BSON("00080018.Value" <<
                                  SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dopamine::services::StoreGenerator generator("");

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::InstanceCreationDate, dcmtkpp::Element({"20150101"}, dcmtkpp::VR::DA));
    dataset.add(dcmtkpp::registry::InstanceCreationTime, dcmtkpp::Element({"101010"}, dcmtkpp::VR::TM));
    dataset.add(dcmtkpp::registry::SOPClassUID, dcmtkpp::Element({dcmtkpp::registry::MRImageStorage}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SOPInstanceUID, dcmtkpp::Element({SOP_INSTANCE_UID_03_01_01_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::AcquisitionDateTime, dcmtkpp::Element({"20150101101010.203"}, dcmtkpp::VR::DT));
    dataset.add(dcmtkpp::registry::QueryRetrieveLevel, dcmtkpp::Element({"STUDY"}, dcmtkpp::VR::CS));
    dataset.add(dcmtkpp::registry::RetrieveAETitle, dcmtkpp::Element({"LOCAL"}, dcmtkpp::VR::AE));
    dataset.add(dcmtkpp::registry::Modality, dcmtkpp::Element({"MR"}, dcmtkpp::VR::CS));
    dataset.add(dcmtkpp::registry::Manufacturer, dcmtkpp::Element({"Manufacturer"}, dcmtkpp::VR::LO));
    dataset.add(dcmtkpp::registry::InstitutionAddress, dcmtkpp::Element({"value"}, dcmtkpp::VR::ST));
    dataset.add(dcmtkpp::registry::SimpleFrameList, dcmtkpp::Element({22}, dcmtkpp::VR::UL));
    dataset.add(dcmtkpp::registry::FailureReason, dcmtkpp::Element({42}, dcmtkpp::VR::US));
    dataset.add(dcmtkpp::registry::StageNumber, dcmtkpp::Element({12}, dcmtkpp::VR::IS));
    dataset.add(dcmtkpp::registry::RecommendedDisplayFrameRateInFloat, dcmtkpp::Element({42.5}, dcmtkpp::VR::FL));
    dataset.add(dcmtkpp::registry::PatientName, dcmtkpp::Element({"Name^Surname^Middle"}, dcmtkpp::VR::PN));

    dcmtkpp::DataSet sequence;
    sequence.add(dcmtkpp::registry::PatientID, dcmtkpp::Element({"123"}, dcmtkpp::VR::LO));
    dataset.add(dcmtkpp::registry::OtherPatientIDsSequence, dcmtkpp::Element({sequence}, dcmtkpp::VR::SQ));

    dataset.add(dcmtkpp::registry::PatientAge, dcmtkpp::Element({"25Y"}, dcmtkpp::VR::AS));
    dataset.add(dcmtkpp::registry::PatientWeight, dcmtkpp::Element({11.11}, dcmtkpp::VR::DS));
    dataset.add(dcmtkpp::registry::EthnicGroup, dcmtkpp::Element({"value"}, dcmtkpp::VR::SH));
    dataset.add(dcmtkpp::registry::AdditionalPatientHistory, dcmtkpp::Element({"value"}, dcmtkpp::VR::LT));
    dataset.add(dcmtkpp::registry::ReferencePixelX0, dcmtkpp::Element({32}, dcmtkpp::VR::SL));
    dataset.add(dcmtkpp::registry::TagAngleSecondAxis, dcmtkpp::Element({32}, dcmtkpp::VR::SS));
    dataset.add(dcmtkpp::registry::StudyInstanceUID, dcmtkpp::Element({STUDY_INSTANCE_UID_03_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SeriesInstanceUID, dcmtkpp::Element({SERIES_INSTANCE_UID_03_01_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::PixelDataProviderURL, dcmtkpp::Element({"value"}, dcmtkpp::VR::UR));
    dataset.add(dcmtkpp::registry::PupilSize, dcmtkpp::Element({42.5}, dcmtkpp::VR::FD));

    // Binary
    dcmtkpp::Value::Binary value = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
    dataset.add(dcmtkpp::registry::ICCProfile, dcmtkpp::Element(value, dcmtkpp::VR::OB));

    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, dcmtkpp::Response::Pending);

    cursor = connection.query(db_name + ".datasets",
                              BSON("00080018.Value" <<
                                   SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK(response.hasField("00080012"));
    BOOST_CHECK(response.hasField("00080013"));
    BOOST_CHECK(response.hasField("00080016"));
    BOOST_CHECK(response.hasField("00080018"));
    BOOST_CHECK(response.hasField("0008002a"));
    BOOST_CHECK(response.hasField("00080052"));
    BOOST_CHECK(response.hasField("00080054"));
    BOOST_CHECK(response.hasField("00080060"));
    BOOST_CHECK(response.hasField("00080070"));
    BOOST_CHECK(response.hasField("00080081"));
    BOOST_CHECK(response.hasField("00081161"));
    BOOST_CHECK(response.hasField("00081197"));
    BOOST_CHECK(response.hasField("00082122"));
    BOOST_CHECK(response.hasField("00089459"));
    BOOST_CHECK(response.hasField("00100010"));
    BOOST_CHECK(response.hasField("00101002"));
    BOOST_CHECK(response.hasField("00101010"));
    BOOST_CHECK(response.hasField("00101030"));
    BOOST_CHECK(response.hasField("00102160"));
    BOOST_CHECK(response.hasField("001021b0"));
    BOOST_CHECK(response.hasField("00186020"));
    BOOST_CHECK(response.hasField("00189219"));
    BOOST_CHECK(response.hasField("0020000d"));
    BOOST_CHECK(response.hasField("0020000e"));
    BOOST_CHECK(response.hasField("00287fe0"));
    BOOST_CHECK(response.hasField("00460044"));
    BOOST_CHECK(response.hasField("Content")); // Dataset is stored in this field

    // The binary tags are not stored
    BOOST_CHECK(response.hasField("00282000") == false);

    BOOST_CHECK_EQUAL(cursor->more(), false);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Perform Store with user constraint
 */
BOOST_FIXTURE_TEST_CASE(Match_Constraint, TestDataGenerator_constraint)
{
    mongo::unique_ptr<mongo::DBClientCursor> cursor =
            this->connection.query(this->db_name + ".datasets",
                              BSON("00080018.Value" <<
                                   SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dopamine::services::StoreGenerator generator("root");

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPClassUID, dcmtkpp::Element({dcmtkpp::registry::MRImageStorage}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SOPInstanceUID, dcmtkpp::Element({SOP_INSTANCE_UID_03_01_01_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::Modality, dcmtkpp::Element({"MR"}, dcmtkpp::VR::CS));
    dataset.add(dcmtkpp::registry::StudyInstanceUID, dcmtkpp::Element({STUDY_INSTANCE_UID_03_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SeriesInstanceUID, dcmtkpp::Element({SERIES_INSTANCE_UID_03_01_01}, dcmtkpp::VR::UI));

    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, dcmtkpp::Response::Pending);

    cursor = this->connection.query(this->db_name + ".datasets",
                              BSON("00080018.Value" <<
                                   SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK(response.hasField("00080018"));
    BOOST_CHECK(response.hasField("00080060"));
    BOOST_CHECK(response.hasField("0020000d"));
    BOOST_CHECK(response.hasField("0020000e"));
    BOOST_CHECK(response.hasField("Content")); // Dataset is stored in this field

    BOOST_CHECK_EQUAL(cursor->more(), false);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Connection with database is failed
 */
BOOST_FIXTURE_TEST_CASE(No_Database_Connection, TestDataGenerator_badconnection)
{
    dopamine::services::StoreGenerator generator("");
    Uint16 result = generator.process_dataset(dcmtkpp::DataSet(), true);
    BOOST_CHECK_EQUAL(result, 0xa700);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: User is not allow to perform query
 */
BOOST_FIXTURE_TEST_CASE(No_Authorization, TestDataGenerator_notallow)
{
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPClassUID, dcmtkpp::Element({dcmtkpp::registry::MRImageStorage}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SOPInstanceUID, dcmtkpp::Element({SOP_INSTANCE_UID_03_01_01_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::StudyInstanceUID, dcmtkpp::Element({STUDY_INSTANCE_UID_03_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SeriesInstanceUID, dcmtkpp::Element({SERIES_INSTANCE_UID_03_01_01}, dcmtkpp::VR::UI));

    dopamine::services::StoreGenerator generator("");
    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, 0xa700);

    dopamine::services::StoreGenerator generator_root("root");
    result = generator_root.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, 0xa700);

    dopamine::services::StoreGenerator generator_notme("not_me");
    result = generator_notme.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, 0xa700);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Missing mandatory field SOPInstanceUID
 */
BOOST_FIXTURE_TEST_CASE(No_SOPInstanceUID, ServicesTestClass)
{
    dopamine::services::StoreGenerator generator("");

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPClassUID, dcmtkpp::Element({dcmtkpp::registry::MRImageStorage}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::StudyInstanceUID, dcmtkpp::Element({STUDY_INSTANCE_UID_03_01}, dcmtkpp::VR::UI));
    dataset.add(dcmtkpp::registry::SeriesInstanceUID, dcmtkpp::Element({SERIES_INSTANCE_UID_03_01_01}, dcmtkpp::VR::UI));

    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, 0xa700);
}
