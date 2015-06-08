/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleQueryRetrieveGenerator
#include <boost/test/unit_test.hpp>

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
        this->add_constraint(dopamine::services::Service_Store, "root", builder.obj());
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
        dopamine::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);
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
    BOOST_CHECK_EQUAL(generator.get_callingaptitle(), "");
    BOOST_CHECK_EQUAL(generator.get_dataset() == NULL, true);
    BOOST_CHECK_EQUAL(generator.is_allow(), false);

    DcmDataset* dataset = new DcmDataset();

    // Setter
    generator.set_callingaptitle("LOCAL");
    BOOST_CHECK_EQUAL(generator.get_callingaptitle(), "LOCAL");

    delete dataset;
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
                             BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dopamine::services::StoreGenerator generator("");

    OFCondition condition = EC_Normal;
    DcmDataset* dataset = new DcmDataset();
    condition = dataset->putAndInsertOFStringArray(DCM_SOPInstanceUID,
                                                   OFString(SOP_INSTANCE_UID_03_01_01_01.c_str()));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_StudyInstanceUID,
                                                   OFString(STUDY_INSTANCE_UID_03_01.c_str()));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_SeriesInstanceUID,
                                                   OFString(SERIES_INSTANCE_UID_03_01_01.c_str()));
    BOOST_REQUIRE(condition.good());

    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    cursor = connection.query(db_name + ".datasets",
                              BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK_EQUAL(response.hasField("00080018"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000d"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000e"), true);

    BOOST_CHECK_EQUAL(cursor->more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Insert all attribute VR
 */
BOOST_FIXTURE_TEST_CASE(Insert_All_VR, ServicesTestClass)
{
    mongo::unique_ptr<mongo::DBClientCursor> cursor =
            connection.query(db_name + ".datasets",
                             BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dopamine::services::StoreGenerator generator("");

    OFCondition condition = EC_Normal;
    DcmDataset* dataset = new DcmDataset();
    condition = dataset->putAndInsertOFStringArray(DCM_InstanceCreationDate, OFString("20150101"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_InstanceCreationTime, OFString("101010"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_SOPClassUID, OFString(UID_MRImageStorage));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_SOPInstanceUID,
                                                   OFString(SOP_INSTANCE_UID_03_01_01_01.c_str()));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_AcquisitionDateTime, OFString("20150101101010.203"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_QueryRetrieveLevel, OFString("STUDY"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_RetrieveAETitle, OFString("LOCAL"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_Modality, OFString("MR"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_Manufacturer, OFString("Manufacturer"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_InstitutionAddress, OFString("value"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertUint32(DCM_SimpleFrameList, 22);
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertUint16(DCM_FailureReason, 42);
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_StageNumber, OFString("12"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertFloat32(DCM_RecommendedDisplayFrameRateInFloat, 42.5);
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_PatientName, OFString("Name^Surname^Middle"));
    BOOST_REQUIRE(condition.good());
    DcmItem* item = NULL;
    condition = dataset->findOrCreateSequenceItem(DCM_OtherPatientIDsSequence, item, -2);
    BOOST_REQUIRE(condition.good());
    condition = item->putAndInsertOFStringArray(DCM_PatientID, "123");
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_PatientAge, OFString("25Y"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_PatientWeight, OFString("11.11"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_EthnicGroup, OFString("value"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_AdditionalPatientHistory, OFString("value"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertSint32(DCM_ReferencePixelX0, 32);
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertSint16(DCM_TagAngleSecondAxis, 32);
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_StudyInstanceUID,
                                                   OFString(STUDY_INSTANCE_UID_03_01.c_str()));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_SeriesInstanceUID,
                                                   OFString(SERIES_INSTANCE_UID_03_01_01.c_str()));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_PixelDataProviderURL, OFString("value"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertFloat64(DCM_PupilSize, 42.5);
    BOOST_REQUIRE(condition.good());
    // Binary
    std::vector<Uint8> value = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
    condition = dataset->putAndInsertUint8Array(DCM_ICCProfile, &value[0], 8);
    BOOST_REQUIRE(condition.good());

    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    cursor = connection.query(db_name + ".datasets",
                              BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
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
    BOOST_CHECK(response.hasField("00282000") == false); // the binary tags are not stored
    BOOST_CHECK(response.hasField("00287fe0"));
    BOOST_CHECK(response.hasField("00460044"));
    BOOST_CHECK(response.hasField("Content")); // Dataset is stored in this field

    BOOST_CHECK_EQUAL(cursor->more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Perform Store with user constraint
 */
BOOST_FIXTURE_TEST_CASE(Match_Constraint, TestDataGenerator_constraint)
{
    mongo::unique_ptr<mongo::DBClientCursor> cursor =
            this->connection.query(this->db_name + ".datasets",
                              BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dopamine::services::StoreGenerator generator("root");

    OFCondition condition = EC_Normal;
    DcmDataset* dataset = new DcmDataset();
    condition = dataset->putAndInsertOFStringArray(DCM_SOPInstanceUID,
                                                   OFString(SOP_INSTANCE_UID_03_01_01_01.c_str()));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_Modality, OFString("MR"));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_StudyInstanceUID,
                                                   OFString(STUDY_INSTANCE_UID_03_01.c_str()));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_SeriesInstanceUID,
                                                   OFString(SERIES_INSTANCE_UID_03_01_01.c_str()));
    BOOST_REQUIRE(condition.good());

    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    cursor = this->connection.query(this->db_name + ".datasets",
                              BSON("00080018.Value" << SOP_INSTANCE_UID_03_01_01_01));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK(response.hasField("00080018"));
    BOOST_CHECK(response.hasField("00080060"));
    BOOST_CHECK(response.hasField("0020000d"));
    BOOST_CHECK(response.hasField("0020000e"));
    BOOST_CHECK(response.hasField("Content")); // Dataset is stored in this field

    BOOST_CHECK_EQUAL(cursor->more(), false);

    delete dataset;
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Connection with database is failed
 */
BOOST_FIXTURE_TEST_CASE(No_Database_Connection, TestDataGenerator_badconnection)
{
    DcmDataset* dataset = new DcmDataset();
    dopamine::services::StoreGenerator generator("");
    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, STATUS_STORE_Refused_OutOfResources);

    delete dataset;
}

/*************************** TEST Error *********************************/
/**
 * Error test case: User is not allow to perform query
 */
BOOST_FIXTURE_TEST_CASE(No_Authorization, TestDataGenerator_notallow)
{
    OFCondition condition = EC_Normal;
    DcmDataset* dataset = new DcmDataset();
    condition = dataset->putAndInsertOFStringArray(DCM_SOPInstanceUID,
                                                   OFString(SOP_INSTANCE_UID_03_01_01_01.c_str()));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_StudyInstanceUID,
                                                   OFString(STUDY_INSTANCE_UID_03_01.c_str()));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_SeriesInstanceUID,
                                                   OFString(SERIES_INSTANCE_UID_03_01_01.c_str()));
    BOOST_REQUIRE(condition.good());

    dopamine::services::StoreGenerator generator("");
    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, STATUS_STORE_Refused_OutOfResources);

    dopamine::services::StoreGenerator generator_root("root");
    result = generator_root.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, STATUS_STORE_Refused_OutOfResources);

    dopamine::services::StoreGenerator generator_notme("not_me");
    result = generator_notme.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, STATUS_STORE_Refused_OutOfResources);

    delete dataset;
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Missing mandatory field SOPInstanceUID
 */
BOOST_FIXTURE_TEST_CASE(No_SOPInstanceUID, ServicesTestClass)
{
    dopamine::services::StoreGenerator generator("");

    OFCondition condition = EC_Normal;
    DcmDataset* dataset = new DcmDataset();
    condition = dataset->putAndInsertOFStringArray(DCM_StudyInstanceUID,
                                                   OFString(STUDY_INSTANCE_UID_03_01.c_str()));
    BOOST_REQUIRE(condition.good());
    condition = dataset->putAndInsertOFStringArray(DCM_SeriesInstanceUID,
                                                   OFString(SERIES_INSTANCE_UID_03_01_01.c_str()));
    BOOST_REQUIRE(condition.good());

    Uint16 result = generator.process_dataset(dataset, true);
    BOOST_CHECK_EQUAL(result, STATUS_FIND_Refused_OutOfResources);

    delete dataset;
}
