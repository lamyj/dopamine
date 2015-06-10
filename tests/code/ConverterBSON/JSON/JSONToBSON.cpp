/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleJSONToBSON
#include <boost/test/unit_test.hpp>

#include <mongo/bson/bson.h>

#include "ConverterBSON/JSON/JSONToBSON.h"

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::converterBSON::JSONToBSON * jsontobson =
            new dopamine::converterBSON::JSONToBSON();

    BOOST_CHECK_EQUAL(jsontobson != NULL, true);

    delete jsontobson;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR AE
 */
BOOST_AUTO_TEST_CASE(ConversionAE)
{
    // Create BSON with AE tag
    std::string const tag = "00080054";
    std::string const vr = "AE";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::JSONToBSON jsontobson;
    mongo::BSONObj const bsonobject = jsontobson.from_json(object);

    BOOST_CHECK_EQUAL(object == bsonobject, true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR DS
 */
BOOST_AUTO_TEST_CASE(ConversionDS)
{
    // Create BSON with DS tag
    std::string const tag = "00101030";
    std::string const vr = "DS";
    mongo::BSONArray const values = BSON_ARRAY(11.11 << 22.22);
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::JSONToBSON jsontobson;
    mongo::BSONObj const bsonobject = jsontobson.from_json(object);

    BOOST_CHECK_EQUAL(object == bsonobject, true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR OB
 */
BOOST_AUTO_TEST_CASE(ConversionOB)
{
    // Create BSON with OB tag
    std::string const tag = "00282000";
    std::string const vr = "OB";
    std::string const value = "YXplcnR5dWlvcHFzZGZnaGprbG13eGN2Ym4xMjM0NTY";
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr <<
                                             "InlineBinary" << value));

    // Conversion
    dopamine::converterBSON::JSONToBSON jsontobson;
    mongo::BSONObj const bsonobject = jsontobson.from_json(object);

    // Check result
    std::string const result = "azertyuiopqsdfghjklmwxcvbn123456";
    mongo::BSONObjBuilder binary_data_builder;
    binary_data_builder.appendBinData("data", result.size(),
                                      mongo::BinDataGeneral,
                                      (void*)(result.c_str()));
    mongo::BSONObj objectcontrol =
            BSON(tag <<
                 BSON("vr" << vr <<
                      "InlineBinary" << binary_data_builder.obj().getField("data")));

    BOOST_CHECK_EQUAL(objectcontrol == bsonobject, true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR PN
 */
BOOST_AUTO_TEST_CASE(ConversionPN)
{
    // Create BSON with SH tag
    std::string const tag = "00100010";
    std::string const vr = "PN";
    mongo::BSONArray const values =
            BSON_ARRAY(BSON("Alphabetic" << "Doe^John^Wallas^Rev.^Chief Executive Officer")
                    << BSON("Alphabetic" << "Smith^Jane^Scarlett^Ms.^Goddess"));
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::JSONToBSON jsontobson;
    mongo::BSONObj const bsonobject = jsontobson.from_json(object);

    BOOST_CHECK_EQUAL(object == bsonobject, true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR SQ
 */
BOOST_AUTO_TEST_CASE(ConversionSQ)
{
    // Create 2 BSON with LO and CS tags
    std::string const tagLO = "00100020";
    std::string const vrLO = "LO";
    mongo::BSONArray const valuesLO = BSON_ARRAY("valueLO1" << "valueLO2");
    std::string const tagCS = "00100022";
    std::string const vrCS = "CS";
    mongo::BSONArray const valuesCS = BSON_ARRAY("valueCS1" << "valueCS2");
    mongo::BSONObj object_1 = BSON(tagLO << BSON("vr" << vrLO <<
                                                 "Value" << valuesLO)
                                << tagCS << BSON("vr" << vrCS <<
                                                 "Value" << valuesCS));

    mongo::BSONObj object_2 = BSON(tagLO << BSON("vr" << vrLO <<
                                                 "Value" << valuesLO)
                                << tagCS << BSON("vr" << vrCS <<
                                                 "Value" << valuesCS));

    // Create BSON with SQ tag
    std::string const tag = "00101002";
    std::string const vr = "SQ";
    mongo::BSONArray const values =
            BSON_ARRAY(object_1 << object_2);
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::JSONToBSON jsontobson;
    mongo::BSONObj const bsonobject = jsontobson.from_json(object);

    BOOST_CHECK_EQUAL(object == bsonobject, true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion Null
 */
BOOST_AUTO_TEST_CASE(ConversionNull)
{
    // Create BSON with AE tag
    std::string const tag = "00080054";
    std::string const vr = "AE";
    mongo::BSONObjBuilder builder;
    builder.appendNull(tag);
    mongo::BSONObj object = builder.obj();

    // Conversion
    dopamine::converterBSON::JSONToBSON jsontobson;
    mongo::BSONObj const bsonobject = jsontobson.from_json(object);

    BOOST_CHECK_EQUAL(object == bsonobject, true);

    mongo::BSONObjBuilder builder2;
    builder2 << "vr" << vr;
    builder2.appendNull("Value");
    object = BSON(tag << builder2.obj());

    // Conversion
    mongo::BSONObj const bsonobject2 = jsontobson.from_json(object);

    BOOST_CHECK_EQUAL(object == bsonobject2, true);
}

