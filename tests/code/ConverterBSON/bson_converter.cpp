/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE Bson_converter
#include <boost/test/unit_test.hpp>

#include <odil/json_converter.h>

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
                       odil::Value::Binary const & expected_value)
{

    BOOST_REQUIRE(!object.isSimpleType());
    BOOST_REQUIRE(object.isABSONObj());
    BOOST_REQUIRE_EQUAL(object.Array().size(), expected_value.size());
    auto const array = object.Array();
    for(unsigned int i=0; i<array.size(); ++i)
    {
        auto const & bson_item = array[i];
        auto const & expected_item = expected_value[i];

        int size=0;
        char const * const begin = bson_item.binDataClean(size);
        odil::Value::Binary::value_type const item(begin, begin+size);
        BOOST_REQUIRE(item==expected_item);
    }
}

BOOST_AUTO_TEST_CASE(AsBSONEmpty)
{
    odil::DataSet data_set;
    auto const bson = dopamine::as_bson(data_set);
    BOOST_REQUIRE(bson.isEmpty());
}

BOOST_AUTO_TEST_CASE(AsBSONEmptyField)
{
    odil::DataSet data_set;
    data_set.add(0xdeadbeef, odil::Value::Strings({}), odil::VR::CS);
    auto const bson = dopamine::as_bson(data_set);

    check_bson_object(bson, {"deadbeef"});
    mongo::BSONElement const element = bson.getField("deadbeef");
    BOOST_REQUIRE(element.isABSONObj());
    check_bson_object(element.Obj(), {"vr"});

    check_bson_string(element.Obj().getField("vr"), "CS");
}

BOOST_AUTO_TEST_CASE(AsBSONIntegers)
{
    odil::DataSet data_set;
    data_set.add(0xdeadbeef, odil::Value::Integers({1, 2}), odil::VR::SS);
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

BOOST_AUTO_TEST_CASE(AsBSONReals)
{
    odil::DataSet data_set;
    data_set.add(0xdeadbeef, odil::Value::Reals({1.2, 3.4}), odil::VR::FL);
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

BOOST_AUTO_TEST_CASE(AsBSONStrings)
{
    odil::DataSet data_set;
    data_set.add(0xdeadbeef, odil::Value::Strings({"FOO", "BAR"}), odil::VR::CS);
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

BOOST_AUTO_TEST_CASE(AsBSONPersonName)
{
    odil::DataSet data_set;
    data_set.add(
        0xdeadbeef,
        odil::Value::Strings({"Alpha^Betic=Ideo^Graphic=Pho^Netic"}),
        odil::VR::PN);
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

BOOST_AUTO_TEST_CASE(AsBSONDataSets)
{
    odil::DataSet item;
    item.add(0xbeeff00d, odil::Value::Integers({1,2}), odil::VR::SS);
    odil::DataSet data_set;
    data_set.add(0xdeadbeef, odil::Value::DataSets({item}), odil::VR::SQ);
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

BOOST_AUTO_TEST_CASE(AsBSONBinary)
{
    odil::DataSet data_set;
    data_set.add(
        0xdeadbeef,
        odil::Value::Binary({{0x1, 0x2, 0x3}, {0x4, 0x5}}), odil::VR::OB);
    auto const bson = dopamine::as_bson(data_set);

    check_bson_object(bson, {"deadbeef"});
    mongo::BSONElement const element = bson.getField("deadbeef");
    BOOST_REQUIRE(element.isABSONObj());
    check_bson_object(element.Obj(), {"vr", "InlineBinary"});
    check_bson_string(element.Obj().getField("vr"), "OB");

    check_bson_binary(element.Obj().getField("InlineBinary"),
                      data_set.as_binary(0xdeadbeef));
}

BOOST_AUTO_TEST_CASE(AsDataSetEmpty)
{
    mongo::BSONObj bson;

    odil::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(data_set.empty());
}

BOOST_AUTO_TEST_CASE(AsDataSetIntegers)
{
    mongo::BSONObj const bson = BSON("deadbeef" <<
                               BSON("vr" << "SS" <<
                                    "Value" << BSON_ARRAY(1 << (long long)2 <<
                                                          (double)5 )));

    odil::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(!data_set.empty());

    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == odil::VR::SS);
    BOOST_REQUIRE(data_set.is_int("deadbeef"));
    BOOST_REQUIRE(data_set.as_int("deadbeef") ==
                  odil::Value::Integers({1, 2, 5}));
}

BOOST_AUTO_TEST_CASE(AsDataSetReals)
{
    mongo::BSONObj const bson = BSON("deadbeef" <<
                               BSON("vr" << "FL" <<
                                    "Value" << BSON_ARRAY(12.34 << 56.78)));

    odil::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(!data_set.empty());

    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == odil::VR::FL);
    BOOST_REQUIRE(data_set.is_real("deadbeef"));
    BOOST_REQUIRE(data_set.as_real("deadbeef") ==
                  odil::Value::Reals({12.34, 56.78}));
}

BOOST_AUTO_TEST_CASE(AsDataSetStrings)
{
    mongo::BSONObj const bson = BSON("deadbeef" << BSON("vr" << "CS" <<
                                                  "Value" << BSON_ARRAY("FOO" <<
                                                                        "BAR")));

    odil::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(!data_set.empty());

    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == odil::VR::CS);
    BOOST_REQUIRE(data_set.is_string("deadbeef"));
    BOOST_REQUIRE(data_set.as_string("deadbeef") ==
                  odil::Value::Strings({"FOO", "BAR"}));
}

BOOST_AUTO_TEST_CASE(AsDataSetPersonName)
{
    mongo::BSONObj const bson =
            BSON("deadbeef" << BSON("vr" << "PN" <<
                                    "Value" << BSON_ARRAY(
                                        BSON("Alphabetic" << "Alpha^Betic" <<
                                             "Ideographic" << "Ideo^Graphic" <<
                                             "Phonetic" << "Pho^Netic"))));

    odil::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(!data_set.empty());

    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == odil::VR::PN);
    BOOST_REQUIRE(data_set.is_string("deadbeef"));
    BOOST_REQUIRE(data_set.as_string("deadbeef") == odil::Value::Strings(
        {"Alpha^Betic=Ideo^Graphic=Pho^Netic"}));
}

BOOST_AUTO_TEST_CASE(AsDataSetDataSets)
{
    mongo::BSONObj const bson =
            BSON("deadbeef" << BSON("vr" << "SQ" <<
                                    "Value" << BSON_ARRAY(
            BSON("beeff00d" << BSON("vr" << "SS" <<
                                    "Value" << BSON_ARRAY(1))))));

    odil::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE(!data_set.empty());

    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == odil::VR::SQ);
    BOOST_REQUIRE(data_set.is_data_set("deadbeef"));

    odil::DataSet item;
    item.add(0xbeeff00d,
        odil::Element(odil::Value::Integers({1}), odil::VR::SS));
    BOOST_REQUIRE(data_set.as_data_set("deadbeef") ==
                  odil::Value::DataSets({item}));
}

BOOST_AUTO_TEST_CASE(AsDataSetBinary)
{
    // Create BSON with OW tag
    mongo::BSONArrayBuilder array_builder;

    {
        std::vector<uint8_t> item = { 0x1, 0x2, 0x3};
        mongo::BSONObjBuilder item_builder;
        item_builder.appendBinData(
            "data", item.size(), mongo::BinDataGeneral, &item[0]);
        array_builder.append(item_builder.obj()["data"]);
    }
    {
        std::vector<uint8_t> item = { 0x4, 0x5};
        mongo::BSONObjBuilder item_builder;
        item_builder.appendBinData(
            "data", item.size(), mongo::BinDataGeneral, &item[0]);
        array_builder.append(item_builder.obj()["data"]);
    }

    mongo::BSONObj const bson = BSON(
        "deadbeef" << BSON(
            "vr" << "OW" <<
            "InlineBinary" << array_builder.arr()));

    odil::DataSet const data_set = dopamine::as_dataset(bson);
    BOOST_REQUIRE_EQUAL(data_set.size(), 1);
    BOOST_REQUIRE(data_set.has("deadbeef"));
    BOOST_REQUIRE(data_set.get_vr("deadbeef") == odil::VR::OW);
    BOOST_REQUIRE(data_set.is_binary("deadbeef"));
    BOOST_REQUIRE(
        data_set.as_binary("deadbeef") ==
        odil::Value::Binary({{0x1, 0x2, 0x3}, {0x4, 0x5}}));
}
