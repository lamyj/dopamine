/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleMongoDBConnection
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/message/Response.h>

#include "../services/ServicesTestClass.h"
#include "core/ConfigurationPACS.h"
#include "dbconnection/MongoDBConnection.h"

struct MongoDBConnectionTest
{
    std::string db_name = "";
    std::string db_host = "";
    int db_port = -1;
    std::vector<std::string> indexeslist;

    MongoDBConnectionTest()
    {
        // Load configuration
        dopamine::ConfigurationPACS::
            get_instance().parse(ServicesTestClass::
                                 _get_env_variable("DOPAMINE_TEST_CONFIG"));

        // Get configuration for Database connection
        dopamine::ConfigurationPACS::get_instance().
                get_database_configuration(db_name, db_host,
                                           db_port, indexeslist);
    }

    ~MongoDBConnectionTest()
    {
        dopamine::ConfigurationPACS::delete_instance();
    }

    void set_authorization(dopamine::MongoDBConnection & connection,
                           std::string const & service, std::string const & user,
                           mongo::BSONObj const & constraint = mongo::BSONObj())
    {
        mongo::BSONObj value = BSON("principal_name" << user <<
                                    "principal_type" << "" <<
                                    "service" << service <<
                                    "dataset" << constraint);
        connection.get_connection().update(
                    connection.get_db_name() + ".authorization",
                    BSON("service" << service), value);
        std::string result =
                connection.get_connection().getLastError(
                    connection.get_db_name());
        if (result != "") // empty string if no error
        {
            BOOST_FAIL(result);
        }
    }
};

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    // Default constructor
    dopamine::MongoDBConnection * connection = new dopamine::MongoDBConnection();
    BOOST_REQUIRE(connection != NULL);
    delete connection; connection = NULL;

    connection = new dopamine::MongoDBConnection("db_name", "localhost",
                                                 104, {});
    BOOST_REQUIRE(connection != NULL);
    delete connection;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    dopamine::MongoDBConnection connection;

    BOOST_REQUIRE(!connection.get_connection().isFailed());

    BOOST_REQUIRE_EQUAL(connection.get_db_name(), "");
    BOOST_REQUIRE_EQUAL(connection.get_host_name(), "localhost");
    BOOST_REQUIRE_EQUAL(connection.get_port(), -1);
    BOOST_REQUIRE_EQUAL(connection.get_indexes().size(), 0);

    connection.set_db_name("mydb");
    BOOST_REQUIRE_EQUAL(connection.get_db_name(), "mydb");

    connection.set_host_name("myaddress");
    BOOST_REQUIRE_EQUAL(connection.get_host_name(), "myaddress");

    connection.set_port(104);
    BOOST_REQUIRE_EQUAL(connection.get_port(), 104);

    connection.set_indexes({"index1", "index2"});
    BOOST_REQUIRE_EQUAL(connection.get_indexes().size(), 2);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Connection
 */
BOOST_FIXTURE_TEST_CASE(Connection, MongoDBConnectionTest)
{
    // Create connection with Database
    dopamine::MongoDBConnection connection(db_name, db_host,
                                           db_port, indexeslist);
    BOOST_REQUIRE(connection.connect());

    // Bad connection
    dopamine::MongoDBConnection badconnection;
    BOOST_REQUIRE(!badconnection.connect());

    dopamine::MongoDBConnection badconnection2(db_name, db_host,
                                        db_port+1, indexeslist);
    BOOST_REQUIRE(!badconnection2.connect());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Command as_string
 */
BOOST_AUTO_TEST_CASE(CommandAsString)
{
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_ECHO_RQ),
                        "Echo");
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_ECHO_RSP),
                        "Echo");
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_FIND_RQ),
                        "Query");
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_FIND_RSP),
                        "Query");
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_GET_RQ),
                        "Retrieve");
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_GET_RSP),
                        "Retrieve");
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_MOVE_RQ),
                        "Retrieve");
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_MOVE_RSP),
                        "Retrieve");
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_STORE_RQ),
                        "Store");
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_STORE_RSP),
                        "Store");
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::as_string(
                            dcmtkpp::message::Message::Command::C_CANCEL_RQ),
                        "");
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: BSON::Element as_string
 */
BOOST_AUTO_TEST_CASE(ElementAsString)
{
    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::
                        as_string(BSON("key" << "value").getField("key")),
                        "value");

    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::
                        as_string(BSON("key" << 111).getField("key")),
                        "111");

    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::
                        as_string(BSON("key" << 11.1).getField("key")),
                        "11.1");

    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::
                        as_string(BSON("key" << (long long)11).getField("key")),
                        "11");

    BOOST_REQUIRE_EQUAL(dopamine::MongoDBConnection::
                        as_string(BSON("key" <<
                                       BSON_ARRAY("value")).getField("key")),
                        "[ \"value\" ]");
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Is_authorized
 */
BOOST_FIXTURE_TEST_CASE(IsAuthorized, MongoDBConnectionTest)
{
    // Create connection with Database
    dopamine::MongoDBConnection connection(db_name, db_host,
                                           db_port, indexeslist);
    BOOST_REQUIRE(connection.connect());

    BOOST_CHECK(!connection.is_authorized
                ("user", dcmtkpp::message::Message::Command::C_ECHO_RQ));

    set_authorization(connection, "Echo", "user", mongo::BSONObj());

    BOOST_CHECK(connection.is_authorized
                ("user", dcmtkpp::message::Message::Command::C_ECHO_RQ));

    set_authorization(connection, "Echo", "", mongo::BSONObj());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Get constraints
 */
BOOST_FIXTURE_TEST_CASE(GetConstraints, MongoDBConnectionTest)
{
    // Create connection with Database
    dopamine::MongoDBConnection connection(db_name, db_host,
                                           db_port, indexeslist);
    BOOST_REQUIRE(connection.connect());

    mongo::BSONObj constraint = connection.get_constraints(
                "", dcmtkpp::message::Message::Command::C_ECHO_RQ);

    BOOST_CHECK(constraint == mongo::BSONObj());

    set_authorization(connection, "Echo", "user", BSON("00100010" << "John"));

    constraint = connection.get_constraints(
                    "user", dcmtkpp::message::Message::Command::C_ECHO_RQ);

    BOOST_CHECK(constraint ==
                BSON("$or" <<
                     BSON_ARRAY(BSON("$and" <<
                                     BSON_ARRAY(BSON("00100010.Value" <<
                                                     "John"))))));

    mongo::BSONObjBuilder builderRegEx;
    builderRegEx.appendRegex("00100010", "J?hn");
    set_authorization(connection, "Echo", "user", builderRegEx.obj());

    constraint = connection.get_constraints(
                    "user", dcmtkpp::message::Message::Command::C_ECHO_RQ);

    mongo::BSONObjBuilder expectedbuilderRegEx;
    expectedbuilderRegEx.appendRegex("00100010.Value", "J?hn");
    BOOST_CHECK(constraint ==
                BSON("$or" <<
                     BSON_ARRAY(BSON("$and" <<
                                     BSON_ARRAY(expectedbuilderRegEx.obj())))));

    constraint = connection.get_constraints(
                    "nouser", dcmtkpp::message::Message::Command::C_ECHO_RQ);

    BOOST_CHECK(constraint ==
                BSON("00080018.Value" <<
                     BSON_ARRAY("_db8eeea6_e0eb_48b8_9a02_a94926b76992")));

    set_authorization(connection, "Echo", "", mongo::BSONObj());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: get_datasets_cursor
 */
BOOST_FIXTURE_TEST_CASE(GetDatasetsCursor, MongoDBConnectionTest)
{
    // Create connection with Database
    dopamine::MongoDBConnection connection(db_name, db_host,
                                           db_port, indexeslist);
    BOOST_REQUIRE(connection.connect());

    auto cursor = connection.get_datasets_cursor();

    BOOST_CHECK(cursor->more());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: run_command
 */
BOOST_FIXTURE_TEST_CASE(RunCommand, MongoDBConnectionTest)
{
    // Create connection with Database
    dopamine::MongoDBConnection connection(db_name, db_host,
                                           db_port, indexeslist);
    BOOST_REQUIRE(connection.connect());

    mongo::BSONObj command =
            BSON("count" << "datasets" << "query"
                 << BSON("00080018.Value" <<
                         BSON_ARRAY("1.2.3")));

    mongo::BSONObj info;
    bool result = connection.run_command(command, info);
    BOOST_REQUIRE(result);

    BOOST_CHECK(info == BSON("n" << 0.0 << "ok" << 1.0));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: insert_dataset
 */
BOOST_FIXTURE_TEST_CASE(InsertDataset, MongoDBConnectionTest)
{
    // Create connection with Database
    dopamine::MongoDBConnection connection(db_name, db_host,
                                           db_port, indexeslist);
    BOOST_REQUIRE(connection.connect());

    mongo::BSONObj command =
            BSON("count" << "datasets" << "query"
                 << BSON("00080018.Value" <<
                         BSON_ARRAY("1.2.3")));

    mongo::BSONObj info;
    bool result = connection.run_command(command, info);
    BOOST_REQUIRE(result);
    BOOST_CHECK(info == BSON("n" << 0.0 << "ok" << 1.0));

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPClassUID, {"1.2.3.4"}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"1.2.3"}, dcmtkpp::VR::UI);
    auto status = connection.insert_dataset("", dataset, "myaet");
    BOOST_REQUIRE(status == dcmtkpp::message::Response::Success);

    result = connection.run_command(command, info);
    BOOST_REQUIRE(result);
    BOOST_CHECK(info == BSON("n" << 1.0 << "ok" << 1.0));

    connection.get_connection().remove(
                connection.get_db_name() + ".datasets",
                BSON("00080018.Value" << "1.2.3"));

    set_authorization(connection, "Store", "user", BSON("00080018" << "4.5.6"));

    status = connection.insert_dataset("user", dataset, "myaet");
    BOOST_REQUIRE(status == dcmtkpp::message::Response::RefusedNotAuthorized);

    mongo::BSONObjBuilder auth;
    auth.appendRegex("00080016", "1.*");
    auth.appendElements(BSON("00080018" << "1.2.3"));
    mongo::BSONObj objauth = auth.obj();
    set_authorization(connection, "Store", "user", objauth);

    status = connection.insert_dataset("user", dataset, "myaet");
    BOOST_REQUIRE(status == dcmtkpp::message::Response::Success);

    connection.get_connection().remove(
                connection.get_db_name() + ".datasets",
                BSON("00080018.Value" << "1.2.3"));

    set_authorization(connection, "Store", "", mongo::BSONObj());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: insert_dataset with big size
 */
BOOST_FIXTURE_TEST_CASE(InsertBigDataset, MongoDBConnectionTest)
{
    // Create connection with Database
    dopamine::MongoDBConnection connection(db_name, db_host,
                                           db_port, indexeslist);
    BOOST_REQUIRE(connection.connect());

    mongo::BSONObj command =
            BSON("count" << "datasets" << "query"
                 << BSON("00080018.Value" <<
                         BSON_ARRAY("1.2.3")));

    mongo::BSONObj info;
    bool result = connection.run_command(command, info);
    BOOST_REQUIRE(result);
    BOOST_CHECK(info == BSON("n" << 0.0 << "ok" << 1.0));

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPClassUID, {"1.2.3.4"}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"1.2.3"}, dcmtkpp::VR::UI);
    dcmtkpp::Value::Binary data; data.resize(16777216);
    dataset.add(dcmtkpp::registry::PixelData, data, dcmtkpp::VR::OW);
    auto status = connection.insert_dataset("", dataset, "myaet");
    BOOST_REQUIRE(status == dcmtkpp::message::Response::Success);

    result = connection.run_command(command, info);
    BOOST_REQUIRE(result);
    BOOST_CHECK(info == BSON("n" << 1.0 << "ok" << 1.0));

    connection.get_connection().remove(
                connection.get_db_name() + ".datasets",
                BSON("00080018.Value" << "1.2.3"));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: get_dataset
 */
BOOST_FIXTURE_TEST_CASE(GetDataset, MongoDBConnectionTest)
{
    // Create connection with Database
    dopamine::MongoDBConnection connection(db_name, db_host,
                                           db_port, indexeslist);
    BOOST_REQUIRE(connection.connect());

    mongo::BSONObj object = BSON("00080016" <<
                                    BSON("vr" << "UI" <<
                                         "Value" << BSON_ARRAY("1.2.3.4")) <<
                                 "00080018" <<
                                    BSON("vr" << "UI" <<
                                         "Value" << BSON_ARRAY("1.2.3")));
    auto data_set = connection.get_dataset(object);
    BOOST_REQUIRE(data_set.first.empty());
    BOOST_REQUIRE(!data_set.second.empty());
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPClassUID));
    BOOST_CHECK_EQUAL(data_set.second.as_string(
                          dcmtkpp::registry::SOPClassUID)[0], "1.2.3.4");
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPInstanceUID));
    BOOST_CHECK_EQUAL(data_set.second.as_string(
                          dcmtkpp::registry::SOPInstanceUID)[0], "1.2.3");

    mongo::BSONObj command =
            BSON("count" << "datasets" << "query"
                 << BSON("00080018.Value" <<
                         BSON_ARRAY("1.2.3")));

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPClassUID, {"1.2.3.4"}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"1.2.3"}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::PatientID, {"my_id"}, dcmtkpp::VR::LO);
    auto status = connection.insert_dataset("", dataset, "myaet");
    BOOST_REQUIRE(status == dcmtkpp::message::Response::Success);

    mongo::BSONObj info;
    bool result = connection.run_command(command, info);
    BOOST_REQUIRE(result);
    BOOST_CHECK(info == BSON("n" << 1.0 << "ok" << 1.0));

    mongo::Query query = BSON("00080018.Value" << "1.2.3");
    mongo::BSONObj fields = BSON("00080018" << 1 << "Content" << 1);
    object = connection.get_connection().findOne(
                connection.get_db_name() + ".datasets", query, &fields);
    data_set = connection.get_dataset(object);
    BOOST_REQUIRE(!data_set.first.empty());
    BOOST_REQUIRE(!data_set.second.empty());
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPClassUID));
    BOOST_CHECK_EQUAL(data_set.second.as_string(
                          dcmtkpp::registry::SOPClassUID)[0], "1.2.3.4");
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPInstanceUID));
    BOOST_CHECK_EQUAL(data_set.second.as_string(
                          dcmtkpp::registry::SOPInstanceUID)[0], "1.2.3");

    connection.get_connection().remove(
                connection.get_db_name() + ".datasets",
                BSON("00080018.Value" << "1.2.3"));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: get_dataset with big size
 */
BOOST_FIXTURE_TEST_CASE(GetBigDataset, MongoDBConnectionTest)
{
    // Create connection with Database
    dopamine::MongoDBConnection connection(db_name, db_host,
                                           db_port, indexeslist);
    BOOST_REQUIRE(connection.connect());

    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPClassUID, {"1.2.3.4"}, dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"1.2.3"}, dcmtkpp::VR::UI);
    dcmtkpp::Value::Binary data; data.resize(16777216);
    dataset.add(dcmtkpp::registry::PixelData, data, dcmtkpp::VR::OW);
    auto status = connection.insert_dataset("", dataset, "myaet");
    BOOST_REQUIRE(status == dcmtkpp::message::Response::Success);

    mongo::BSONObj command =
            BSON("count" << "datasets" << "query"
                 << BSON("00080018.Value" <<
                         BSON_ARRAY("1.2.3")));
    mongo::BSONObj info;
    bool result = connection.run_command(command, info);
    BOOST_REQUIRE(result);
    BOOST_CHECK(info == BSON("n" << 1.0 << "ok" << 1.0));

    mongo::Query query = BSON("00080018.Value" << "1.2.3");
    mongo::BSONObj fields = BSON("00080018" << 1 << "Content" << 1);
    mongo::BSONObj object = connection.get_connection().findOne(
                connection.get_db_name() + ".datasets", query, &fields);
    auto data_set = connection.get_dataset(object);
    BOOST_REQUIRE(!data_set.first.empty());
    BOOST_REQUIRE(!data_set.second.empty());
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPClassUID));
    BOOST_CHECK_EQUAL(data_set.second.as_string(
                          dcmtkpp::registry::SOPClassUID)[0], "1.2.3.4");
    BOOST_REQUIRE(data_set.second.has(dcmtkpp::registry::SOPInstanceUID));
    BOOST_CHECK_EQUAL(data_set.second.as_string(
                          dcmtkpp::registry::SOPInstanceUID)[0], "1.2.3");

    connection.get_connection().remove(
                connection.get_db_name() + ".datasets",
                BSON("00080018.Value" << "1.2.3"));
}
