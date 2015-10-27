/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleBson_converter
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/json_converter.h>

#include "ConverterBSON/bson_converter.h"

template<typename TChecker, typename TGetter, typename TValue>
void check_bson_array(
    mongo::BSONElement const & object, TChecker checker, TGetter getter,
    TValue const & expected)
{
    std::vector<mongo::BSONElement> array = object.Array();
    BOOST_REQUIRE_EQUAL(array.size(), expected.size());

    for(unsigned int i = 0; i < array.size(); ++i)
    {
        BOOST_REQUIRE((array[i].*checker)());
        BOOST_REQUIRE_EQUAL((array[i].*getter)(), expected[i]);
    }
}

void check_bson_object(mongo::BSONObj const & object,
    std::set<std::string> const & expected_members)
{
    BOOST_REQUIRE(object.isValid());
    std::set<std::string> fields;
    auto size = object.getFieldNames(fields);
    BOOST_REQUIRE_EQUAL(size, expected_members.size());

    BOOST_REQUIRE(fields == expected_members);
}

void check_bson_string(mongo::BSONElement const & object,
                       std::string const & expected_value)
{
    BOOST_REQUIRE_EQUAL(object.String(), expected_value);
}

void check_bson_binary(mongo::BSONElement const & object,
                       dcmtkpp::Value::Binary const & expected_value)
{
    BOOST_REQUIRE(!object.isSimpleType());
    BOOST_REQUIRE(!object.isABSONObj());

    int size=0;
    char const * begin = object.binDataClean(size);
    BOOST_REQUIRE_EQUAL(size, expected_value.size());

    dcmtkpp::Value::Binary result;
    result.resize(size);
    std::copy(begin, begin+size, result.begin());

    BOOST_REQUIRE(result == expected_value);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion Empty Dataset
 */
BOOST_AUTO_TEST_CASE(AsBSONEmpty)
{
    dcmtkpp::DataSet data_set;
    auto const bson = dopamine::as_bson(data_set);
    BOOST_REQUIRE(bson.isEmpty());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion Empty Field
 */
BOOST_AUTO_TEST_CASE(AsBSONEmptyField)
{
    dcmtkpp::DataSet data_set;
    data_set.add(0xdeadbeef,
        dcmtkpp::Element(dcmtkpp::Value::Strings({}), dcmtkpp::VR::CS));
    auto const bson = dopamine::as_bson(data_set);

    check_bson_object(bson, {"deadbeef"});
    mongo::BSONElement const element = bson.getField("deadbeef");
    BOOST_REQUIRE(element.isABSONObj());
    check_bson_object(element.Obj(), {"vr"});

    check_bson_string(element.Obj().getField("vr"), "CS");
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion BSON Integers
 */
BOOST_AUTO_TEST_CASE(AsBSONIntegers)
{
    dcmtkpp::DataSet data_set;
    data_set.add(0xdeadbeef,
        dcmtkpp::Element(dcmtkpp::Value::Integers({1, 2}), dcmtkpp::VR::SS));
    auto const bson = dopamine::as_bson(data_set);

    check_bson_object(bson, {"deadbeef"});
    mongo::BSONElement const element = bson.getField("deadbeef");
    BOOST_REQUIRE(element.isABSONObj());
    check_bson_object(element.Obj(), {"vr", "Value"});
    check_bson_string(element.Obj().getField("vr"), "SS");
    check_bson_array(element.Obj().getField("Value"),
                     &mongo::BSONElement::isNumber,
                     &mongo::BSONElement::Long,
                     data_set.as_int(0xdeadbeef));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion BSON Reals
 */
BOOST_AUTO_TEST_CASE(AsBSONReals)
{
    dcmtkpp::DataSet data_set;
    data_set.add(0xdeadbeef,
        dcmtkpp::Element(dcmtkpp::Value::Reals({1.2, 3.4}), dcmtkpp::VR::FL));
    auto const bson = dopamine::as_bson(data_set);

    check_bson_object(bson, {"deadbeef"});
    mongo::BSONElement const element = bson.getField("deadbeef");
    BOOST_REQUIRE(element.isABSONObj());
    check_bson_object(element.Obj(), {"vr", "Value"});
    check_bson_string(element.Obj().getField("vr"), "FL");
    check_bson_array(element.Obj().getField("Value"),
                     &mongo::BSONElement::isNumber,
                     &mongo::BSONElement::Double,
                     data_set.as_real(0xdeadbeef));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion BSON Strings
 */
BOOST_AUTO_TEST_CASE(AsBSONStrings)
{
    dcmtkpp::DataSet data_set;
    data_set.add(0xdeadbeef,
        dcmtkpp::Element(
            dcmtkpp::Value::Strings({"FOO", "BAR"}),
            dcmtkpp::VR::CS));
    auto const bson = dopamine::as_bson(data_set);

    check_bson_object(bson, {"deadbeef"});
    mongo::BSONElement const element = bson.getField("deadbeef");
    BOOST_REQUIRE(element.isABSONObj());
    check_bson_object(element.Obj(), {"vr", "Value"});
    check_bson_string(element.Obj().getField("vr"), "CS");
    check_bson_array(element.Obj().getField("Value"),
                     &mongo::BSONElement::isSimpleType,
                     &mongo::BSONElement::String,
                     data_set.as_string(0xdeadbeef));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion BSON Strings PN
 */
BOOST_AUTO_TEST_CASE(AsBSONPersonName)
{
    dcmtkpp::DataSet data_set;
    data_set.add(0xdeadbeef,
        dcmtkpp::Element(
            dcmtkpp::Value::Strings({"Alpha^Betic=Ideo^Graphic=Pho^Netic"}),
            dcmtkpp::VR::PN));
    auto const bson = dopamine::as_bson(data_set);

    check_bson_object(bson, {"deadbeef"});
    mongo::BSONElement const element = bson.getField("deadbeef");
    BOOST_REQUIRE(element.isABSONObj());
    check_bson_object(element.Obj(), {"vr", "Value"});
    check_bson_string(element.Obj().getField("vr"), "PN");

    BOOST_REQUIRE(element.Obj().getField("Value").isABSONObj());
    std::vector<mongo::BSONElement> array =
            element.Obj().getField("Value").Array();
    BOOST_REQUIRE_EQUAL(array.size(), 1);
    check_bson_object(
        array[0].Obj(), {"Alphabetic", "Ideographic", "Phonetic"});
    check_bson_string(array[0].Obj().getField("Alphabetic"), {"Alpha^Betic"});
    check_bson_string(array[0].Obj().getField("Ideographic"), {"Ideo^Graphic"});
    check_bson_string(array[0].Obj().getField("Phonetic"), {"Pho^Netic"});
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion BSON Sequence
 */
BOOST_AUTO_TEST_CASE(AsBSONDataSets)
{
    dcmtkpp::DataSet item;
    item.add(0xbeeff00d,
        dcmtkpp::Element(dcmtkpp::Value::Integers({1,2}), dcmtkpp::VR::SS));
    dcmtkpp::DataSet data_set;
    data_set.add(0xdeadbeef,
        dcmtkpp::Element(
            dcmtkpp::Value::DataSets({item}),
            dcmtkpp::VR::SQ));
    auto const bson = dopamine::as_bson(data_set);

    check_bson_object(bson, {"deadbeef"});
    mongo::BSONElement const element = bson.getField("deadbeef");
    BOOST_REQUIRE(element.isABSONObj());
    check_bson_object(element.Obj(), {"vr", "Value"});
    check_bson_string(element.Obj().getField("vr"), "SQ");

    BOOST_REQUIRE(element.Obj().getField("Value").isABSONObj());
    std::vector<mongo::BSONElement> array =
            element.Obj().getField("Value").Array();
    BOOST_REQUIRE_EQUAL(array.size(), 1);
    check_bson_object(array[0].Obj(), {"beeff00d"});
    mongo::BSONElement const elementsq = array[0].Obj().getField("beeff00d");
    BOOST_REQUIRE(elementsq.isABSONObj());

    check_bson_object(elementsq.Obj(), {"vr", "Value"});
    check_bson_string(elementsq.Obj().getField("vr"), "SS");
    check_bson_array(elementsq.Obj().getField("Value"),
                     &mongo::BSONElement::isNumber,
                     &mongo::BSONElement::Long,
                     item.as_int(0xbeeff00d));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion BSON Binary
 */
BOOST_AUTO_TEST_CASE(AsBSONBinary)
{
    dcmtkpp::DataSet data_set;
    data_set.add(0xdeadbeef,
        dcmtkpp::Element(
            dcmtkpp::Value::Binary({0x1, 0x2, 0x3, 0x4, 0x5}),
            dcmtkpp::VR::OB));
    auto const bson = dopamine::as_bson(data_set);

    check_bson_object(bson, {"deadbeef"});
    mongo::BSONElement const element = bson.getField("deadbeef");
    BOOST_REQUIRE(element.isABSONObj());
    check_bson_object(element.Obj(), {"vr", "InlineBinary"});
    check_bson_string(element.Obj().getField("vr"), "OB");

    check_bson_binary(element.Obj().getField("InlineBinary"),
                      data_set.as_binary(0xdeadbeef));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion Dataset Empty
 */
BOOST_AUTO_TEST_CASE(AsDataSetEmpty)
{
    mongo::BSONObj bson;

    dcmtkpp::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(data_set.empty());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion Dataset Integers
 */
BOOST_AUTO_TEST_CASE(AsDataSetIntegers)
{
    mongo::BSONObj const bson = BSON("deadbeef" <<
                               BSON("vr" << "SS" <<
                                    "Value" << BSON_ARRAY(1 << (long long)2 <<
                                                          (double)5 )));

    dcmtkpp::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(!data_set.empty());

    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == dcmtkpp::VR::SS);
    BOOST_REQUIRE(data_set.is_int("deadbeef"));
    BOOST_REQUIRE(data_set.as_int("deadbeef") ==
                  dcmtkpp::Value::Integers({1, 2, 5}));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion Dataset Reals
 */
BOOST_AUTO_TEST_CASE(AsDataSetReals)
{
    mongo::BSONObj const bson = BSON("deadbeef" <<
                               BSON("vr" << "FL" <<
                                    "Value" << BSON_ARRAY(12.34 << 56.78)));

    dcmtkpp::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(!data_set.empty());

    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == dcmtkpp::VR::FL);
    BOOST_REQUIRE(data_set.is_real("deadbeef"));
    BOOST_REQUIRE(data_set.as_real("deadbeef") ==
                  dcmtkpp::Value::Reals({12.34, 56.78}));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion Dataset Strings
 */
BOOST_AUTO_TEST_CASE(AsDataSetStrings)
{
    mongo::BSONObj const bson = BSON("deadbeef" << BSON("vr" << "CS" <<
                                                  "Value" << BSON_ARRAY("FOO" <<
                                                                        "BAR")));

    dcmtkpp::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(!data_set.empty());

    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == dcmtkpp::VR::CS);
    BOOST_REQUIRE(data_set.is_string("deadbeef"));
    BOOST_REQUIRE(data_set.as_string("deadbeef") ==
                  dcmtkpp::Value::Strings({"FOO", "BAR"}));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion Dataset Strings PN
 */
BOOST_AUTO_TEST_CASE(AsDataSetPersonName)
{
    mongo::BSONObj const bson =
            BSON("deadbeef" << BSON("vr" << "PN" <<
                                    "Value" << BSON_ARRAY(
                                        BSON("Alphabetic" << "Alpha^Betic" <<
                                             "Ideographic" << "Ideo^Graphic" <<
                                             "Phonetic" << "Pho^Netic"))));

    dcmtkpp::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(!data_set.empty());

    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == dcmtkpp::VR::PN);
    BOOST_REQUIRE(data_set.is_string("deadbeef"));
    BOOST_REQUIRE(data_set.as_string("deadbeef") == dcmtkpp::Value::Strings(
        {"Alpha^Betic=Ideo^Graphic=Pho^Netic"}));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion Dataset Sequences
 */
BOOST_AUTO_TEST_CASE(AsDataSetDataSets)
{
    mongo::BSONObj const bson =
            BSON("deadbeef" << BSON("vr" << "SQ" <<
                                    "Value" << BSON_ARRAY(
            BSON("beeff00d" << BSON("vr" << "SS" <<
                                    "Value" << BSON_ARRAY(1))))));

    dcmtkpp::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(!data_set.empty());

    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == dcmtkpp::VR::SQ);
    BOOST_REQUIRE(data_set.is_data_set("deadbeef"));

    dcmtkpp::DataSet item;
    item.add(0xbeeff00d,
        dcmtkpp::Element(dcmtkpp::Value::Integers({1}), dcmtkpp::VR::SS));
    BOOST_REQUIRE(data_set.as_data_set("deadbeef") ==
                  dcmtkpp::Value::DataSets({item}));
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Conversion Dataset Binary
 */
BOOST_AUTO_TEST_CASE(AsDataSetBinary)
{
    // Create BSON with OW tag
    std::vector<uint8_t> value = { 0x1, 0x2, 0x3, 0x4, 0x5 };
    mongo::BSONObjBuilder binary_data_builder;
    binary_data_builder.appendBinData("data", value.size(),
                                      mongo::BinDataGeneral,
                                      (void*)(&value[0]));
    mongo::BSONObj const bson =
            BSON("deadbeef" << BSON("vr" << "OW" <<
                                    "InlineBinary" <<
                            binary_data_builder.obj().getField("data")));

    dcmtkpp::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == dcmtkpp::VR::OW);
    BOOST_REQUIRE(data_set.is_binary("deadbeef"));
    BOOST_REQUIRE(data_set.as_binary("deadbeef") == dcmtkpp::Value::Binary(
        {0x1, 0x2, 0x3, 0x4, 0x5}));
}
