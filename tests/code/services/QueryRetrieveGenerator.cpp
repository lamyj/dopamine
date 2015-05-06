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
#include "services/QueryRetrieveGenerator.h"
#include "services/ServicesTools.h"

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

        mongo::BSONObj query_value = BSON("principal_name" << "root" <<
                                          "principal_type" << "" <<
                                          "service" << "Query" <<
                                          "dataset" << mongo::BSONObj());
        _connection.update(_db_name + ".authorization", BSON("service" << "Query"), query_value);
        mongo::BSONObj retrieve_value = BSON("principal_name" << "root" <<
                                             "principal_type" << "" <<
                                             "service" << "Retrieve" <<
                                             "dataset" << mongo::BSONObj());
        _connection.update(_db_name + ".authorization", BSON("service" << "Retrieve"), retrieve_value);
    }

    ~TestDataGenerator_notallow()
    {
        mongo::BSONObj query_value = BSON("principal_name" << "" <<
                                          "principal_type" << "" <<
                                          "service" << "Query" <<
                                          "dataset" << mongo::BSONObj());
        _connection.update(_db_name + ".authorization", BSON("service" << "Query"), query_value);
        mongo::BSONObj retrieve_value = BSON("principal_name" << "" <<
                                             "principal_type" << "" <<
                                             "service" << "Retrieve" <<
                                             "dataset" << mongo::BSONObj());
        _connection.update(_db_name + ".authorization", BSON("service" << "Retrieve"), retrieve_value);

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
    dopamine::services::QueryRetrieveGenerator * generator_query =
            new dopamine::services::QueryRetrieveGenerator("", dopamine::services::Service_Query);

    BOOST_CHECK_EQUAL(generator_query != NULL, true);

    delete generator_query;

    dopamine::services::QueryRetrieveGenerator * generator_retrieve =
            new dopamine::services::QueryRetrieveGenerator("", dopamine::services::Service_Retrieve);

    BOOST_CHECK_EQUAL(generator_retrieve != NULL, true);

    delete generator_retrieve;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Getter and Setter
 */
BOOST_FIXTURE_TEST_CASE(Accessors, TestDataGenerator)
{
    // Same for Service_Query and Service_Retrieve
    dopamine::services::QueryRetrieveGenerator generator("", dopamine::services::Service_Query);

    // Default initialization
    BOOST_CHECK_EQUAL(generator.get_instance_count_tags().size(), 0);
    BOOST_CHECK_EQUAL(generator.get_convert_modalities_in_study(), false);
    BOOST_CHECK_EQUAL(generator.get_query_retrieve_level(), "");
    BOOST_CHECK_EQUAL(generator.get_maximumResults(), 0);
    BOOST_CHECK_EQUAL(generator.get_skippedResults(), 0);
    BOOST_CHECK_EQUAL(generator.get_fuzzymatching(), false);
    BOOST_CHECK_EQUAL(generator.is_allow(), false);

    // Setter
    generator.set_maximumResults(5);
    BOOST_CHECK_EQUAL(generator.get_maximumResults(), 5);
    generator.set_skippedResults(2);
    BOOST_CHECK_EQUAL(generator.get_skippedResults(), 2);
    generator.set_fuzzymatching(true);
    BOOST_CHECK_EQUAL(generator.get_fuzzymatching(), true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Function Cancel
 */
BOOST_FIXTURE_TEST_CASE(Cancel, TestDataGenerator)
{
    // Same for Service_Query and Service_Retrieve
    dopamine::services::QueryRetrieveGenerator generator("", dopamine::services::Service_Query);

    // Not yet implemented
    generator.cancel();
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Empty request
 */
BOOST_FIXTURE_TEST_CASE(Empty_Request, TestDataGenerator)
{
    // Service QUERY
    dopamine::services::QueryRetrieveGenerator generator_query("", dopamine::services::Service_Query);

    // STUDY
    mongo::BSONObj query = BSON("00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("STUDY")));
    Uint16 result = generator_query.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    mongo::BSONObj findedobject = generator_query.next();
    BOOST_CHECK_EQUAL(findedobject.isEmpty(), false);
    BOOST_CHECK_EQUAL(findedobject.hasField("$err"), true);

    // SERIES
    query = BSON("00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("SERIES")));
    result = generator_query.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    findedobject = generator_query.next();
    BOOST_CHECK_EQUAL(findedobject.isEmpty(), false);
    BOOST_CHECK_EQUAL(findedobject.hasField("$err"), true);

    // IMAGE
    query = BSON("00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("IMAGE")));
    result = generator_query.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    findedobject = generator_query.next();
    BOOST_CHECK_EQUAL(findedobject.isEmpty(), false);
    BOOST_CHECK_EQUAL(findedobject.hasField("$err"), true);

    // Service RETRIEVE
    dopamine::services::QueryRetrieveGenerator generator_retrieve("", dopamine::services::Service_Retrieve);

    // STUDY
    query = BSON("00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("STUDY")));
    result = generator_retrieve.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    findedobject = generator_retrieve.next();
    BOOST_CHECK_EQUAL(findedobject.isEmpty(), false);
    BOOST_CHECK_EQUAL(findedobject.hasField("$err"), true);

    // SERIES
    query = BSON("00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("SERIES")));
    result = generator_retrieve.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    findedobject = generator_retrieve.next();
    BOOST_CHECK_EQUAL(findedobject.isEmpty(), false);
    BOOST_CHECK_EQUAL(findedobject.hasField("$err"), true);

    // IMAGE
    query = BSON("00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("IMAGE")));
    result = generator_retrieve.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    findedobject = generator_retrieve.next();
    BOOST_CHECK_EQUAL(findedobject.isEmpty(), false);
    BOOST_CHECK_EQUAL(findedobject.hasField("$err"), true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Request with matching
 */
BOOST_FIXTURE_TEST_CASE(Request_Match, TestDataGenerator)
{
    mongo::BSONObj const query = BSON("00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("STUDY"))
                                   << "00080060" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("MR")));

    // Service QUERY
    dopamine::services::QueryRetrieveGenerator generator_query("", dopamine::services::Service_Query);

    Uint16 result = generator_query.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    mongo::BSONObj findedobject = generator_query.next();
    unsigned int count = 0;
    while (!findedobject.isEmpty())
    {
        BOOST_CHECK_EQUAL(findedobject.hasField("00080060"), true);
        ++count;
        findedobject = generator_query.next();
    }
    BOOST_CHECK_EQUAL(count, 4);

    // Service RETRIEVE
    dopamine::services::QueryRetrieveGenerator generator_retrieve("", dopamine::services::Service_Retrieve);

    result = generator_retrieve.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    findedobject = generator_retrieve.next();
    count = 0;
    while (!findedobject.isEmpty())
    {
        BOOST_CHECK_EQUAL(findedobject.hasField("00080060"), true);
        ++count;
        findedobject = generator_retrieve.next();
    }
    BOOST_CHECK_EQUAL(count, 4);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Request with no matching (all attributes VR)
 */
BOOST_FIXTURE_TEST_CASE(Request_No_Match, TestDataGenerator)
{
    // Service QUERY
    dopamine::services::QueryRetrieveGenerator generator_query("", dopamine::services::Service_Query);

    // Create binary data for OB, OF, OW, UN
    std::string const value = "azertyuiopqsdfghjklmwxcvbn123456";
    mongo::BSONObjBuilder binary_data_builder;
    binary_data_builder.appendBinData("data", value.size(),
                                      mongo::BinDataGeneral, (void*)(value.c_str()));
    mongo::BSONObj const binary_data = binary_data_builder.obj();

    // TODO: add OB, OF, OW, SQ, UN
    mongo::BSONObjBuilder builder;
    builder << "00080012" << BSON("vr" << "DA" << "Value" << BSON_ARRAY("20150101"))
            << "00080013" << BSON("vr" << "TM" << "Value" << BSON_ARRAY("101010"))
            << "00080016" << BSON("vr" << "UI" << "Value" << BSON_ARRAY("value"))
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
            << "00101010" << BSON("vr" << "AS" << "Value" << BSON_ARRAY("25Y"))
            << "00101030" << BSON("vr" << "DS" << "Value" << BSON_ARRAY("11.11"))
            << "00102160" << BSON("vr" << "SH" << "Value" << BSON_ARRAY("value"))
            << "001021b0" << BSON("vr" << "LT" << "Value" << BSON_ARRAY("value"))
            << "00186020" << BSON("vr" << "SL" << "Value" << BSON_ARRAY("32"))
            << "00189219" << BSON("vr" << "SS" << "Value" << BSON_ARRAY("32"))
            << "00282000" << BSON("vr" << "OB" << "InlineBinary" << binary_data.getField("data"))
            << "00287fe0" << BSON("vr" << "UT" << "Value" << BSON_ARRAY("value"))
            << "00460044" << BSON("vr" << "FD" << "Value" << BSON_ARRAY("42.5"));


    mongo::BSONObj const query = builder.obj();
    Uint16 result = generator_query.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    mongo::BSONObj findedobject = generator_query.next();
    unsigned int count = 0;
    while (!findedobject.isEmpty())
    {
        ++count;
        findedobject = generator_query.next();
    }
    BOOST_CHECK_EQUAL(count, 0);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Match Regex
 */
BOOST_FIXTURE_TEST_CASE(Request_Match_Regex, TestDataGenerator)
{
    // Service QUERY
    dopamine::services::QueryRetrieveGenerator generator_query("", dopamine::services::Service_Query);

    mongo::BSONObjBuilder builder;
    builder << "00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("STUDY"))
            << "00080070" << BSON("vr" << "LO" << "Value" << BSON_ARRAY("Manu?act*"))
            << "00100010" << BSON("vr" << "PN" << "Value" << BSON_ARRAY(BSON("Alphabetic" << "N?me*")));

    mongo::BSONObj const query = builder.obj();
    Uint16 result = generator_query.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    mongo::BSONObj findedobject = generator_query.next();
    unsigned int count = 0;
    while (!findedobject.isEmpty())
    {
        ++count;
        findedobject = generator_query.next();
    }
    BOOST_CHECK_EQUAL(count, 0);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Match Range
 */
BOOST_FIXTURE_TEST_CASE(Request_Match_Range, TestDataGenerator)
{
    // Service QUERY
    dopamine::services::QueryRetrieveGenerator generator_query("", dopamine::services::Service_Query);

    mongo::BSONObjBuilder builder;
    builder << "00080012" << BSON("vr" << "DA" << "Value" << BSON_ARRAY("20150101-201501031"))
            << "00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("STUDY"));

    mongo::BSONObj const query = builder.obj();
    Uint16 result = generator_query.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    mongo::BSONObj findedobject = generator_query.next();
    unsigned int count = 0;
    while (!findedobject.isEmpty())
    {
        ++count;
        findedobject = generator_query.next();
    }
    BOOST_CHECK_EQUAL(count, 0);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Match Null
 */
BOOST_FIXTURE_TEST_CASE(Request_Match_Null, TestDataGenerator)
{
    // Service QUERY
    dopamine::services::QueryRetrieveGenerator generator_query("", dopamine::services::Service_Query);

    mongo::BSONObjBuilder valuebuilder;
    valuebuilder << "vr" << "DA";
    valuebuilder.appendNull("Value");

    mongo::BSONObjBuilder builder;
    builder << "00080012" << valuebuilder.obj()
            << "00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("STUDY"))
            << "00080060" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("NotMR"));

    mongo::BSONObj const query = builder.obj();
    Uint16 result = generator_query.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    mongo::BSONObj findedobject = generator_query.next();
    unsigned int count = 0;
    while (!findedobject.isEmpty())
    {
        ++count;
        findedobject = generator_query.next();
    }
    BOOST_CHECK_EQUAL(count, 0);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Include field in response
 */
BOOST_FIXTURE_TEST_CASE(Request_IncludeField, TestDataGenerator)
{
    // Service QUERY
    dopamine::services::QueryRetrieveGenerator generator_query("", dopamine::services::Service_Query);

    std::vector<std::string> fields_to_get = { "00100010", "00102210"};

    mongo::BSONObjBuilder builder;
    builder << "00080018" << BSON("vr" << "UI" << "Value" << BSON_ARRAY("1.2.276.0.7230010.3.1.4.8323329.4396.1430723598.811431"))
            << "00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY("STUDY"));

    mongo::BSONObj const query = builder.obj();

    // First without fields
    Uint16 result = generator_query.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    mongo::BSONObj findedobject = generator_query.next();
    unsigned int count = 0;
    while (!findedobject.isEmpty())
    {
        ++count;

        for (auto field : fields_to_get)
        {
            BOOST_CHECK_EQUAL(findedobject.hasField(field), false);
        }

        findedobject = generator_query.next();
    }
    BOOST_CHECK_EQUAL(count, 1);

    // Second with fields
    generator_query.set_includefields(fields_to_get);

    result = generator_query.set_query(query);
    BOOST_CHECK_EQUAL(result, STATUS_Pending);

    findedobject = generator_query.next();
    count = 0;
    while (!findedobject.isEmpty())
    {
        ++count;

        for (auto field : fields_to_get)
        {
            BOOST_CHECK_EQUAL(findedobject.hasField(field), true);
        }

        findedobject = generator_query.next();
    }
    BOOST_CHECK_EQUAL(count, 1);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Compute attribute
 */
BOOST_FIXTURE_TEST_CASE(Compute_Attribute, TestDataGenerator)
{
    // Service QUERY
    dopamine::services::QueryRetrieveGenerator generator_query("", dopamine::services::Service_Query);

    // attribute == "00080056"
    mongo::BSONObj object = generator_query.compute_attribute("00080056", "");
    BOOST_CHECK_EQUAL(object.hasField("00080056"), true);
    BOOST_CHECK_EQUAL(object.getField("00080056").Obj().getField("Value").Array()[0].String(), "ONLINE");

    // attribute == "00080061"
    object = generator_query.compute_attribute("00080061",
                                               "2.16.756.5.5.100.1333920868.19866.1424334602.23");
    BOOST_CHECK_EQUAL(object.hasField("00080061"), true);
    BOOST_CHECK_EQUAL(object.getField("00080061").Obj().getField("Value").Array()[0].String(), "MR");

    // attribute == "00201200"
    object = generator_query.compute_attribute("00201200",
                                               "id123");
    BOOST_CHECK_EQUAL(object.hasField("00201200"), true);
    BOOST_CHECK_EQUAL(object.getField("00201200").Obj().getField("Value").Array()[0].Int(), 1);

    // attribute == "00201202"
    object = generator_query.compute_attribute("00201202",
                                               "id123");
    BOOST_CHECK_EQUAL(object.hasField("00201202"), true);
    BOOST_CHECK_EQUAL(object.getField("00201202").Obj().getField("Value").Array()[0].Int(), 1);

    // attribute == "00201204"
    object = generator_query.compute_attribute("00201204",
                                               "id123");
    BOOST_CHECK_EQUAL(object.hasField("00201204"), true);
    BOOST_CHECK_EQUAL(object.getField("00201204").Obj().getField("Value").Array()[0].Int(), 3);

    // attribute == "00201206"
    object = generator_query.compute_attribute("00201206",
                                               "2.16.756.5.5.100.1333920868.19866.1424334602.23");
    BOOST_CHECK_EQUAL(object.hasField("00201206"), true);
    BOOST_CHECK_EQUAL(object.getField("00201206").Obj().getField("Value").Array()[0].Int(), 1);

    // attribute == "00201208"
    object = generator_query.compute_attribute("00201208",
                                               "2.16.756.5.5.100.1333920868.19866.1424334602.23");
    BOOST_CHECK_EQUAL(object.hasField("00201208"), true);
    BOOST_CHECK_EQUAL(object.getField("00201208").Obj().getField("Value").Array()[0].Int(), 3);

    // attribute == "00201209"
    object = generator_query.compute_attribute("00201209",
                                               "2.16.756.5.5.100.1333920868.31960.1424338206.1");
    BOOST_CHECK_EQUAL(object.hasField("00201209"), true);
    BOOST_CHECK_EQUAL(object.getField("00201209").Obj().getField("Value").Array()[0].Int(), 3);

    // Bad attribute
    object = generator_query.compute_attribute("00100010", "");
    BOOST_CHECK_EQUAL(object.isEmpty(), true);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Connection with database is failed
 */
BOOST_FIXTURE_TEST_CASE(No_Database_Connection, TestDataGenerator_badconnection)
{
    // Service Query
    dopamine::services::QueryRetrieveGenerator generator_query("", dopamine::services::Service_Query);
    Uint16 result = generator_query.set_query(mongo::BSONObj());
    BOOST_CHECK_EQUAL(result, STATUS_FIND_Refused_OutOfResources);

    // Service Retrieve
    dopamine::services::QueryRetrieveGenerator generator_retrieve("", dopamine::services::Service_Retrieve);
    result = generator_retrieve.set_query(mongo::BSONObj());
    BOOST_CHECK_EQUAL(result, STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: User is not allow to perform query
 */
BOOST_FIXTURE_TEST_CASE(No_Authorization, TestDataGenerator_notallow)
{
    // Service Query
    dopamine::services::QueryRetrieveGenerator generator_query("not_root", dopamine::services::Service_Query);
    Uint16 result = generator_query.set_query(mongo::BSONObj());
    BOOST_CHECK_EQUAL(result, STATUS_FIND_Refused_OutOfResources);

    // Service Retrieve
    dopamine::services::QueryRetrieveGenerator generator_retrieve("not_root", dopamine::services::Service_Retrieve);
    result = generator_retrieve.set_query(mongo::BSONObj());
    BOOST_CHECK_EQUAL(result, STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Missing mandatory field QueryRetrieveLevel
 */
BOOST_FIXTURE_TEST_CASE(No_QueryRetrieveLevel, TestDataGenerator)
{
    // Service Query
    dopamine::services::QueryRetrieveGenerator generator_query("", dopamine::services::Service_Query);
    Uint16 result = generator_query.set_query(mongo::BSONObj());
    BOOST_CHECK_EQUAL(result, STATUS_FIND_Refused_OutOfResources);

    // Service Retrieve
    dopamine::services::QueryRetrieveGenerator generator_retrieve("", dopamine::services::Service_Retrieve);
    result = generator_retrieve.set_query(mongo::BSONObj());
    BOOST_CHECK_EQUAL(result, STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches);
}
