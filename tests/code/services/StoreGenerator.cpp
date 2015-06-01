/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleQueryRetrieveGenerator
#include <boost/test/unit_test.hpp>

#include "core/ConfigurationPACS.h"
#include "services/StoreGenerator.h"
#include "services/ServicesTools.h"

std::string const UNIQUE_SOP_INSTANCE_UID = "UID_721730bd_8b5e_4fa7_a9e8_e8975330091a";
std::string const UNIQUE_STUDY_INSTANCE_UID = "UID_ddf656ca_24c3_47dc_9152_d7ed2d384162";
std::string const UNIQUE_SERIES_INSTANCE_UID = "UID_9a66f8cd_1c44_48e9_af6b_fe78282b741c";

struct TestDataGenerator
{
    TestDataGenerator()
    {
        std::string NetworkConfFILE(getenv("DOPAMINE_TEST_CONFIG"));
        dopamine::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);
    }

    ~TestDataGenerator()
    {
        dopamine::ConfigurationPACS::delete_instance();
        sleep(1);
    }
};

struct TestDataGenerator_constraint
{
    mongo::DBClientConnection _connection;
    std::string _db_name;

    TestDataGenerator_constraint()
    {
        std::string NetworkConfFILE(getenv("DOPAMINE_TEST_CONFIG"));
        dopamine::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);

        dopamine::services::create_db_connection(_connection, _db_name);

        mongo::BSONObjBuilder builder;
        builder << "00080060" << "MR";
        mongo::BSONObj store_value = BSON("principal_name" << "root" <<
                                          "principal_type" << "" <<
                                          "service" << "Store" <<
                                          "dataset" << builder.obj());
        _connection.insert(_db_name + ".authorization",
                           store_value);
    }

    ~TestDataGenerator_constraint()
    {
        _connection.remove(_db_name + ".authorization", BSON("principal_name" << "root"));

        dopamine::ConfigurationPACS::delete_instance();
        sleep(1);
    }
};

struct TestDataGenerator_badconnection
{
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

struct TestDataGenerator_notallow
{
    mongo::DBClientConnection _connection;
    std::string _db_name;

    TestDataGenerator_notallow()
    {
        std::string NetworkConfFILE(getenv("DOPAMINE_TEST_CONFIG"));
        dopamine::ConfigurationPACS::get_instance().Parse(NetworkConfFILE);

        dopamine::services::create_db_connection(_connection, _db_name);

        mongo::BSONObjBuilder builder;
        builder.appendRegex("00080018", "Unknown");
        mongo::BSONObj store_value = BSON("principal_name" << "root" <<
                                          "principal_type" << "" <<
                                          "service" << "Store" <<
                                          "dataset" << builder.obj());
        _connection.update(_db_name + ".authorization", BSON("service" << "Store"), store_value);

        mongo::BSONObjBuilder builder2;
        builder2 << "00080060" << "NotMR";
        mongo::BSONObj store_value2 = BSON("principal_name" << "not_me" <<
                                           "principal_type" << "" <<
                                           "service" << "Store" <<
                                           "dataset" << builder2.obj());
        _connection.insert(_db_name + ".authorization",
                           store_value2);
        sleep(1); // Wait for database update
    }

    ~TestDataGenerator_notallow()
    {
        _connection.remove(_db_name + ".authorization", BSON("principal_name" << "not_me"));

        mongo::BSONObj store_value = BSON("principal_name" << "" <<
                                          "principal_type" << "" <<
                                          "service" << "Store" <<
                                          "dataset" << mongo::BSONObj());
        _connection.update(_db_name + ".authorization", BSON("service" << "Store"), store_value);

        dopamine::ConfigurationPACS::delete_instance();
        sleep(1);
    }
};

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_FIXTURE_TEST_CASE(Constructor, TestDataGenerator)
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
BOOST_FIXTURE_TEST_CASE(Accessors, TestDataGenerator)
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
    generator.set_dataset(dataset);
    BOOST_CHECK_EQUAL(generator.get_dataset() == NULL, false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Function Cancel
 */
BOOST_FIXTURE_TEST_CASE(Cancel, TestDataGenerator)
{
    dopamine::services::StoreGenerator generator("");

    // Not yet implemented
    generator.cancel();
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Empty request
 */
BOOST_FIXTURE_TEST_CASE(Empty_Request, TestDataGenerator)
{
    mongo::DBClientConnection connection;
    std::string db_name;
    dopamine::services::create_db_connection(connection, db_name);

    mongo::unique_ptr<mongo::DBClientCursor> cursor =
            connection.query(db_name + ".datasets",
                             BSON("00080018.Value" << UNIQUE_SOP_INSTANCE_UID));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dopamine::services::StoreGenerator generator("");

    mongo::BSONObjBuilder builder;
    builder << "00080018" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SOP_INSTANCE_UID))
            << "0020000d" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_STUDY_INSTANCE_UID))
            << "0020000e" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SERIES_INSTANCE_UID));
    mongo::BSONObj const query = builder.obj();
    Uint16 result = generator.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    sleep(1); // wait for database storage

    cursor = connection.query(db_name + ".datasets",
                              BSON("00080018.Value" << UNIQUE_SOP_INSTANCE_UID));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK_EQUAL(response.hasField("00080018"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000d"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000e"), true);

    BOOST_CHECK_EQUAL(cursor->more(), false);

    connection.remove(db_name + ".datasets",
                      BSON("00080018.Value" << UNIQUE_SOP_INSTANCE_UID));
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Insert all attribute VR
 */
BOOST_FIXTURE_TEST_CASE(Insert_All_VR, TestDataGenerator)
{
    mongo::DBClientConnection connection;
    std::string db_name;
    dopamine::services::create_db_connection(connection, db_name);

    mongo::unique_ptr<mongo::DBClientCursor> cursor =
            connection.query(db_name + ".datasets",
                             BSON("00080018.Value" << UNIQUE_SOP_INSTANCE_UID));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dopamine::services::StoreGenerator generator("");

    // Create binary data for OB, OF, OW, UN
    std::string const value = "azertyuiopqsdfghjklmwxcvbn123456";
    mongo::BSONObjBuilder binary_data_builder;
    binary_data_builder.appendBinData("data", value.size(),
                                      mongo::BinDataGeneral, (void*)(value.c_str()));
    mongo::BSONObj const binary_data = binary_data_builder.obj();

    // Create Item for Sequence
    mongo::BSONObjBuilder objectsequence;
    objectsequence << "00100020" << BSON("vr" << "LO" << "Value" << BSON_ARRAY("12345"));

    mongo::BSONObjBuilder builder;
    builder << "00080012" << BSON("vr" << "DA" << "Value" << BSON_ARRAY("20150101"))
            << "00080013" << BSON("vr" << "TM" << "Value" << BSON_ARRAY("101010"))
            << "00080016" << BSON("vr" << "UI" << "Value" << BSON_ARRAY("value"))
            << "00080018" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SOP_INSTANCE_UID))
            << "0008002a" << BSON("vr" << "DT" << "Value" << BSON_ARRAY("20150101101010.203"))
            << "00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("STUDY"))
            << "00080054" << BSON("vr" << "AE" << "Value" << BSON_ARRAY("LOCAL"))
            << "00080060" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("MR"))
            << "00080070" << BSON("vr" << "LO" << "Value" << BSON_ARRAY("Manufacturer"))
            << "00080081" << BSON("vr" << "ST" << "Value" << BSON_ARRAY("value"))
            << "00081161" << BSON("vr" << "UL" << "Value" << BSON_ARRAY("22"))
            << "00081197" << BSON("vr" << "US" << "Value" << BSON_ARRAY("42"))
            << "00082122" << BSON("vr" << "IS" << "Value" << BSON_ARRAY("12"))
            << "00089459" << BSON("vr" << "FL" << "Value" << BSON_ARRAY("42.5"))
            << "00100010" << BSON("vr" << "PN" << "Value" << BSON_ARRAY(BSON("Alphabetic" << "Name^Surname^Middle")))
            << "00101002" << BSON("vr" << "SQ" << "Value" << BSON_ARRAY(objectsequence.obj()))
            << "00101010" << BSON("vr" << "AS" << "Value" << BSON_ARRAY("25Y"))
            << "00101030" << BSON("vr" << "DS" << "Value" << BSON_ARRAY("11.11"))
            << "00102160" << BSON("vr" << "SH" << "Value" << BSON_ARRAY("value"))
            << "001021b0" << BSON("vr" << "LT" << "Value" << BSON_ARRAY("value"))
            << "00186020" << BSON("vr" << "SL" << "Value" << BSON_ARRAY("32"))
            << "00189219" << BSON("vr" << "SS" << "Value" << BSON_ARRAY("32"))
            << "0020000d" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_STUDY_INSTANCE_UID))
            << "0020000e" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SERIES_INSTANCE_UID))
            << "00282000" << BSON("vr" << "OB" << "InlineBinary" << binary_data.getField("data"))
            << "00287fe0" << BSON("vr" << "UT" << "Value" << BSON_ARRAY("value"))
            << "00460044" << BSON("vr" << "FD" << "Value" << BSON_ARRAY("42.5"));
    mongo::BSONObj const query = builder.obj();
    Uint16 result = generator.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    sleep(1); // wait for database storage

    cursor = connection.query(db_name + ".datasets",
                              BSON("00080018.Value" << UNIQUE_SOP_INSTANCE_UID));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK_EQUAL(response.hasField("00080012"), true);
    BOOST_CHECK_EQUAL(response.hasField("00080013"), true);
    BOOST_CHECK_EQUAL(response.hasField("00080016"), true);
    BOOST_CHECK_EQUAL(response.hasField("00080018"), true);
    BOOST_CHECK_EQUAL(response.hasField("0008002a"), true);
    BOOST_CHECK_EQUAL(response.hasField("00080052"), true);
    BOOST_CHECK_EQUAL(response.hasField("00080054"), true);
    BOOST_CHECK_EQUAL(response.hasField("00080060"), true);
    BOOST_CHECK_EQUAL(response.hasField("00080070"), true);
    BOOST_CHECK_EQUAL(response.hasField("00080081"), true);
    BOOST_CHECK_EQUAL(response.hasField("00081161"), true);
    BOOST_CHECK_EQUAL(response.hasField("00081197"), true);
    BOOST_CHECK_EQUAL(response.hasField("00082122"), true);
    BOOST_CHECK_EQUAL(response.hasField("00089459"), true);
    BOOST_CHECK_EQUAL(response.hasField("00100010"), true);
    BOOST_CHECK_EQUAL(response.hasField("00101002"), true);
    BOOST_CHECK_EQUAL(response.hasField("00101010"), true);
    BOOST_CHECK_EQUAL(response.hasField("00101030"), true);
    BOOST_CHECK_EQUAL(response.hasField("00102160"), true);
    BOOST_CHECK_EQUAL(response.hasField("001021b0"), true);
    BOOST_CHECK_EQUAL(response.hasField("00186020"), true);
    BOOST_CHECK_EQUAL(response.hasField("00189219"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000d"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000e"), true);
    BOOST_CHECK_EQUAL(response.hasField("00282000"), true);
    BOOST_CHECK_EQUAL(response.hasField("00287fe0"), true);
    BOOST_CHECK_EQUAL(response.hasField("00460044"), true);

    BOOST_CHECK_EQUAL(cursor->more(), false);

    connection.remove(db_name + ".datasets",
                      BSON("00080018.Value" << UNIQUE_SOP_INSTANCE_UID));
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Perform Store with user constraint
 */
BOOST_FIXTURE_TEST_CASE(Match_Constraint, TestDataGenerator_constraint)
{
    mongo::unique_ptr<mongo::DBClientCursor> cursor =
            _connection.query(_db_name + ".datasets",
                              BSON("00080018.Value" << UNIQUE_SOP_INSTANCE_UID));
    BOOST_CHECK_EQUAL(cursor->more(), false);

    dopamine::services::StoreGenerator generator("root");

    mongo::BSONObjBuilder builder;
    builder << "00080018" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SOP_INSTANCE_UID))
            << "00080060" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("MR"))
            << "0020000d" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_STUDY_INSTANCE_UID))
            << "0020000e" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SERIES_INSTANCE_UID));
    mongo::BSONObj const query = builder.obj();
    Uint16 result = generator.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    sleep(1); // wait for database storage

    cursor = _connection.query(_db_name + ".datasets",
                              BSON("00080018.Value" << UNIQUE_SOP_INSTANCE_UID));
    BOOST_CHECK_EQUAL(cursor->more(), true);

    mongo::BSONObj response = cursor->next();
    BOOST_CHECK_EQUAL(response.hasField("00080018"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000d"), true);
    BOOST_CHECK_EQUAL(response.hasField("0020000e"), true);

    BOOST_CHECK_EQUAL(cursor->more(), false);

    _connection.remove(_db_name + ".datasets",
                       BSON("00080018.Value" << UNIQUE_SOP_INSTANCE_UID));
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Connection with database is failed
 */
BOOST_FIXTURE_TEST_CASE(No_Database_Connection, TestDataGenerator_badconnection)
{
    dopamine::services::StoreGenerator generator("");
    Uint16 result = generator.set_query(mongo::BSONObj());
    BOOST_CHECK_EQUAL(result, STATUS_STORE_Refused_OutOfResources);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: User is not allow to perform query
 */
BOOST_FIXTURE_TEST_CASE(No_Authorization, TestDataGenerator_notallow)
{
    mongo::BSONObjBuilder builder;
    builder << "00080018" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SOP_INSTANCE_UID))
            << "0020000d" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_STUDY_INSTANCE_UID))
            << "0020000e" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SERIES_INSTANCE_UID));
    mongo::BSONObj const query = builder.obj();

    dopamine::services::StoreGenerator generator("");
    Uint16 result = generator.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_STORE_Refused_OutOfResources);

    dopamine::services::StoreGenerator generator_root("root");
    result = generator_root.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_STORE_Refused_OutOfResources);

    dopamine::services::StoreGenerator generator_notme("not_me");
    result = generator_notme.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_STORE_Refused_OutOfResources);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Missing mandatory field SOPInstanceUID
 */
BOOST_FIXTURE_TEST_CASE(No_SOPInstanceUID, TestDataGenerator)
{
    dopamine::services::StoreGenerator generator("");

    mongo::BSONObjBuilder builder;
    builder << "0020000d" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_STUDY_INSTANCE_UID))
            << "0020000e" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SERIES_INSTANCE_UID));
    mongo::BSONObj const query = builder.obj();
    Uint16 result = generator.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_FIND_Refused_OutOfResources);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Missing mandatory field StudyInstanceUID
 */
BOOST_FIXTURE_TEST_CASE(No_StudyInstanceUID, TestDataGenerator)
{
    dopamine::services::StoreGenerator generator("");

    mongo::BSONObjBuilder builder;
    builder << "00080018" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SOP_INSTANCE_UID))
            << "0020000e" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SERIES_INSTANCE_UID));
    mongo::BSONObj const query = builder.obj();
    Uint16 result = generator.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_FIND_Refused_OutOfResources);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Missing mandatory field SeriesInstanceUID
 */
BOOST_FIXTURE_TEST_CASE(No_SeriesInstanceUID, TestDataGenerator)
{
    dopamine::services::StoreGenerator generator("");

    mongo::BSONObjBuilder builder;
    builder << "00080018" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_SOP_INSTANCE_UID))
            << "0020000d" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(UNIQUE_STUDY_INSTANCE_UID));
    mongo::BSONObj const query = builder.obj();
    Uint16 result = generator.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_FIND_Refused_OutOfResources);
}
