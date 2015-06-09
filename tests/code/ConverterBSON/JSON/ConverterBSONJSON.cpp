/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

/*************************************************************************
 * Be Carefull: this files does not contain unit tests but functionnal
 * tests
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleConvertBSONJSON
#include <boost/test/unit_test.hpp>

#include <ConverterBSON/JSON/BSONToJSON.h>
#include <ConverterBSON/JSON/JSONToBSON.h>

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Converter BSON <-> JSON
 */
struct TestDataConversionBSONJSON
{
   mongo::BSONObj bsonobject;

   TestDataConversionBSONJSON()
   {
       mongo::BSONObjBuilder bsonobjectbuilder;

       // Insert AE
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "AE";
       value_builder << "Value" << BSON_ARRAY("test_AE");
       bsonobjectbuilder << "00080054" << value_builder.obj();
       }

       // Insert AS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "AS";
       value_builder << "Value" << BSON_ARRAY("test_AS");
       bsonobjectbuilder << "00101010" << value_builder.obj();
       }

       // Insert CS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "CS";
       value_builder << "Value" << BSON_ARRAY("value1");
       bsonobjectbuilder << "00080060" << value_builder.obj();
       }

       // Insert DA
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "DA";
       value_builder << "Value" << BSON_ARRAY("01/01/2001");
       bsonobjectbuilder << "00100030" << value_builder.obj();
       }

       // Insert DS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "DS";
       value_builder << "Value" << BSON_ARRAY(60.5);
       bsonobjectbuilder << "00101030" << value_builder.obj();
       }

       // Insert DT
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "DT";
       value_builder << "Value" << BSON_ARRAY("01/01/2001 09:09:09");
       bsonobjectbuilder << "00189074" << value_builder.obj();
       }

       // Insert FD
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "FD";
       value_builder << "Value" << BSON_ARRAY(42.5);
       bsonobjectbuilder << "00460044" << value_builder.obj();
       }

       // Insert FL
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "FL";
       value_builder << "Value" << BSON_ARRAY(15.2);
       bsonobjectbuilder << "00089459" << value_builder.obj();
       }

       // Insert IS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "IS";
       value_builder << "Value" << BSON_ARRAY(12);
       bsonobjectbuilder << "00082122" << value_builder.obj();
       }

       // Insert LO
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "LO";
       value_builder << "Value" << BSON_ARRAY("MyManufacturer");
       bsonobjectbuilder << "00080070" << value_builder.obj();
       }

       // Insert LT
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "LT";
       value_builder << "Value" << BSON_ARRAY("test_valueLT");
       bsonobjectbuilder << "001021b0" << value_builder.obj();
       }

       // Insert PN
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "PN";
       value_builder << "Value" << BSON_ARRAY(BSON("Alphabetic" << "Doe^John"));
       bsonobjectbuilder << "00100010" << value_builder.obj();
       }

       // Insert SH
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SH";
       value_builder << "Value" << BSON_ARRAY("test_valueSH");
       bsonobjectbuilder << "00102160" << value_builder.obj();
       }

       // Insert SL
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SL";
       value_builder << "Value" << BSON_ARRAY(10);
       bsonobjectbuilder << "00186020" << value_builder.obj();
       }

       // Insert SS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SS";
       value_builder << "Value" << BSON_ARRAY(11);
       bsonobjectbuilder << "00189219" << value_builder.obj();
       }

       // Insert ST
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "ST";
       value_builder << "Value" << BSON_ARRAY("MyAddress");
       bsonobjectbuilder << "00080081" << value_builder.obj();
       }

       // Insert TM
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "TM";
       value_builder << "Value" << BSON_ARRAY("08:08:08");
       bsonobjectbuilder << "00080013" << value_builder.obj();
       }

       // Insert UI
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "UI";
       value_builder << "Value" << BSON_ARRAY("1.2.3.4.5.6");
       bsonobjectbuilder << "00080016" << value_builder.obj();
       }

       // Insert UL
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "UL";
       value_builder << "Value" << BSON_ARRAY(6);
       bsonobjectbuilder << "00081161" << value_builder.obj();
       }

       // Insert US
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "US";
       value_builder << "Value" << BSON_ARRAY(5);
       bsonobjectbuilder << "00081197" << value_builder.obj();
       }

       // Insert UT
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "UT";
       value_builder << "Value" << BSON_ARRAY("test_valueUT");
       bsonobjectbuilder << "00287fe0" << value_builder.obj();
       }


       // Insert SQ
       {
       mongo::BSONObjBuilder subvalue_builder;
       subvalue_builder << "vr" << "LO";
       subvalue_builder << "Value" << BSON_ARRAY("123");

       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SQ";
       value_builder << "Value" << BSON_ARRAY(BSON("00100020" << subvalue_builder.obj()));
       bsonobjectbuilder << "00101002" << value_builder.obj();
       }

       bsonobject = bsonobjectbuilder.obj();
   }

   ~TestDataConversionBSONJSON()
   {
       // Nothing to do
   }
};

void isEqual(mongo::BSONObj const & firstbson, mongo::BSONObj const & secondbson)
{
    for(mongo::BSONObj::iterator it = firstbson.begin(); it.more();)
    {
        mongo::BSONElement const element_bson = it.next();

        std::string const field_name = element_bson.fieldName();
        BOOST_CHECK_EQUAL(secondbson.hasField(field_name), true);

        mongo::BSONElement const newbsonelem = secondbson.getField(field_name);

        // Error with double representation
        if (element_bson.Obj().getField("Value").Array()[0].type() == mongo::BSONType::NumberDouble)
        {
            BOOST_CHECK_EQUAL(element_bson.Obj().getField("vr").String(),
                              newbsonelem.Obj().getField("vr").String());
            BOOST_CHECK_CLOSE(element_bson.Obj().getField("Value").Array()[0].Double(),
                              newbsonelem.Obj().getField("Value").Array()[0].Double(),
                              0.001);
        }
        else
        {
            BOOST_CHECK_EQUAL(element_bson == newbsonelem, true);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(ConversionBSONJSON, TestDataConversionBSONJSON)
{
    // Convert original BSON to JSON
    dopamine::converterBSON::BSONToJSON bsontojson;
    mongo::BSONObj const json = bsontojson.to_json(bsonobject);

    // Convert JSON to new BSON
    dopamine::converterBSON::JSONToBSON jsontobson;
    mongo::BSONObj const newbson = jsontobson.from_json(json);

    // Compare original BSON with new
    isEqual(bsonobject, newbson);
    // Compare new BSON with original
    isEqual(newbson, bsonobject);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Converter BSON <-> JSON
 */
BOOST_FIXTURE_TEST_CASE(ConversionBSONJSON_String, TestDataConversionBSONJSON)
{
    // Convert original BSON to JSON
    dopamine::converterBSON::BSONToJSON bsontojson;
    std::string const json = bsontojson.to_string(bsonobject);

    // Convert JSON to new BSON
    dopamine::converterBSON::JSONToBSON jsontobson;
    mongo::BSONObj const newbson = jsontobson.from_string(json);

    // Compare original BSON with new
    isEqual(bsonobject, newbson);
    // Compare new BSON with original
    isEqual(newbson, bsonobject);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Converter JSON <-> BSON
 */
struct TestDataConversionJSONBSON
{
   mongo::BSONObj jsonobject;

   TestDataConversionJSONBSON()
   {
       mongo::BSONObjBuilder bsonobjectbuilder;

       // Insert AE
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "AE";
       value_builder << "Value" << BSON_ARRAY("test_AE");
       bsonobjectbuilder << "00080054" << value_builder.obj();
       }

       // Insert AS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "AS";
       value_builder << "Value" << BSON_ARRAY("test_AS");
       bsonobjectbuilder << "00101010" << value_builder.obj();
       }

       // Insert CS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "CS";
       value_builder << "Value" << BSON_ARRAY("value1");
       bsonobjectbuilder << "00080060" << value_builder.obj();
       }

       // Insert DA
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "DA";
       value_builder << "Value" << BSON_ARRAY("01/01/2001");
       bsonobjectbuilder << "00100030" << value_builder.obj();
       }

       // Insert DS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "DS";
       value_builder << "Value" << BSON_ARRAY(60.5);
       bsonobjectbuilder << "00101030" << value_builder.obj();
       }

       // Insert DT
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "DT";
       value_builder << "Value" << BSON_ARRAY("01/01/2001 09:09:09");
       bsonobjectbuilder << "00189074" << value_builder.obj();
       }

       // Insert FD
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "FD";
       value_builder << "Value" << BSON_ARRAY(42.5);
       bsonobjectbuilder << "00460044" << value_builder.obj();
       }

       // Insert FL
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "FL";
       value_builder << "Value" << BSON_ARRAY(15.2);
       bsonobjectbuilder << "00089459" << value_builder.obj();
       }

       // Insert IS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "IS";
       value_builder << "Value" << BSON_ARRAY(12);
       bsonobjectbuilder << "00082122" << value_builder.obj();
       }

       // Insert LO
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "LO";
       value_builder << "Value" << BSON_ARRAY("MyManufacturer");
       bsonobjectbuilder << "00080070" << value_builder.obj();
       }

       // Insert LT
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "LT";
       value_builder << "Value" << BSON_ARRAY("test_valueLT");
       bsonobjectbuilder << "001021b0" << value_builder.obj();
       }

       // Insert PN
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "PN";
       value_builder << "Value" << BSON_ARRAY(BSON("Alphabetic" << "Doe^John"));
       bsonobjectbuilder << "00100010" << value_builder.obj();
       }

       // Insert SH
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SH";
       value_builder << "Value" << BSON_ARRAY("test_valueSH");
       bsonobjectbuilder << "00102160" << value_builder.obj();
       }

       // Insert SL
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SL";
       value_builder << "Value" << BSON_ARRAY(10);
       bsonobjectbuilder << "00186020" << value_builder.obj();
       }

       // Insert SS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SS";
       value_builder << "Value" << BSON_ARRAY(11);
       bsonobjectbuilder << "00189219" << value_builder.obj();
       }

       // Insert ST
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "ST";
       value_builder << "Value" << BSON_ARRAY("MyAddress");
       bsonobjectbuilder << "00080081" << value_builder.obj();
       }

       // Insert TM
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "TM";
       value_builder << "Value" << BSON_ARRAY("08:08:08");
       bsonobjectbuilder << "00080013" << value_builder.obj();
       }

       // Insert UI
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "UI";
       value_builder << "Value" << BSON_ARRAY("1.2.3.4.5.6");
       bsonobjectbuilder << "00080016" << value_builder.obj();
       }

       // Insert UL
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "UL";
       value_builder << "Value" << BSON_ARRAY(6);
       bsonobjectbuilder << "00081161" << value_builder.obj();
       }

       // Insert US
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "US";
       value_builder << "Value" << BSON_ARRAY(5);
       bsonobjectbuilder << "00081197" << value_builder.obj();
       }

       // Insert UT
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "UT";
       value_builder << "Value" << BSON_ARRAY("test_valueUT");
       bsonobjectbuilder << "00287fe0" << value_builder.obj();
       }


       // Insert SQ
       {
       mongo::BSONObjBuilder subvalue_builder;
       subvalue_builder << "vr" << "LO";
       subvalue_builder << "Value" << BSON_ARRAY("123");

       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SQ";
       value_builder << "Value" << BSON_ARRAY(BSON("00100020" << subvalue_builder.obj()));
       bsonobjectbuilder << "00101002" << value_builder.obj();
       }

       jsonobject = bsonobjectbuilder.obj();
   }

   ~TestDataConversionJSONBSON()
   {
       // Nothing to do
   }
};

BOOST_FIXTURE_TEST_CASE(ConversionJSONBSON, TestDataConversionJSONBSON)
{
    // Convert original BSON to Dataset
    dopamine::converterBSON::JSONToBSON jsontobson;
    mongo::BSONObj const bson_ = jsontobson.from_json(jsonobject);

    // Convert Dataset to new BSON
    dopamine::converterBSON::BSONToJSON bsontojson;
    mongo::BSONObj const newjson = bsontojson.to_json(bson_);

    // Compare original JSON with new
    isEqual(jsonobject, newjson);
    // Compare new JSON with original
    isEqual(newjson, jsonobject);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Converter JSON <-> BSON
 */
BOOST_AUTO_TEST_CASE(ConversionJSONBSON_String)
{
    std::stringstream jsonstream;
    jsonstream << "{ "
               << "\"00080054\" : { \"vr\" : \"AE\", \"Value\" : [ \"valueAE\" ] }, "
               << "\"00101010\" : { \"vr\" : \"AS\", \"Value\" : [ \"valueAS\" ] }, "
               << "\"00080060\" : { \"vr\" : \"CS\", \"Value\" : [ \"valueCS\" ] }, "
               << "\"00100030\" : { \"vr\" : \"DA\", \"Value\" : [ \"valueDA\" ] }, "
               << "\"00101030\" : { \"vr\" : \"DS\", \"Value\" : [ 60.5 ] }, "
               << "\"00189074\" : { \"vr\" : \"DT\", \"Value\" : [ \"valueDT\" ] }, "
               << "\"00460044\" : { \"vr\" : \"FD\", \"Value\" : [ 42.5 ] }, "
               << "\"00089459\" : { \"vr\" : \"FL\", \"Value\" : [ 15.2 ] }, "
               << "\"00082122\" : { \"vr\" : \"IS\", \"Value\" : [ 12 ] }, "
               << "\"00080070\" : { \"vr\" : \"LO\", \"Value\" : [ \"valueLO\" ] }, "
               << "\"001021b0\" : { \"vr\" : \"LT\", \"Value\" : [ \"valueLT\" ] }, "
               << "\"00100010\" : { \"vr\" : \"PN\", \"Value\" : [ { \"Alphabetic\" : \"valueAE\" } ] }, "
               << "\"00102160\" : { \"vr\" : \"SH\", \"Value\" : [ \"valueSH\" ] }, "
               << "\"00186020\" : { \"vr\" : \"SL\", \"Value\" : [ 10 ] }, "
               << "\"00189219\" : { \"vr\" : \"SS\", \"Value\" : [ 11 ] }, "
               << "\"00080081\" : { \"vr\" : \"ST\", \"Value\" : [ \"valueST\" ] }, "
               << "\"00080013\" : { \"vr\" : \"TM\", \"Value\" : [ \"valueTM\" ] }, "
               << "\"00080016\" : { \"vr\" : \"UI\", \"Value\" : [ \"valueUI\" ] }, "
               << "\"00081161\" : { \"vr\" : \"UL\", \"Value\" : [ 6 ] }, "
               << "\"00081197\" : { \"vr\" : \"US\", \"Value\" : [ 7 ] }, "
               << "\"00287fe0\" : { \"vr\" : \"UT\", \"Value\" : [ \"valueUT\" ] }, "
               << "\"00101002\" : { \"vr\" : \"SQ\", \"Value\" : [ { \"00100020\" : { \"vr\" : \"LO\", \"Value\" : [ \"valueLO\" ] } } ] }"
               << " }";

    // Convert original JSON to BSON
    dopamine::converterBSON::JSONToBSON jsontobson;
    mongo::BSONObj const bson_ = jsontobson.from_string(jsonstream.str());

    // Convert BSON to new JSON
    dopamine::converterBSON::BSONToJSON bsontojson;
    std::string const newjson = bsontojson.to_string(bson_);

    BOOST_CHECK_EQUAL(jsonstream.str(), newjson);
}
