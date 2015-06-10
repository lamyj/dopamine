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

#define BOOST_TEST_MODULE ModuleConverterBSONXML
#include <boost/property_tree/xml_parser.hpp>
#include <boost/test/unit_test.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/ofstd/oftypes.h>

#include <mongo/bson/bson.h>

#include "ConverterBSON/XML/BSONToXML.h"
#include "ConverterBSON/XML/XMLToBSON.h"

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Converter BSON <-> XML
 */
struct TestDataConversionBSONXML
{
   mongo::BSONObj bsonobject;

   TestDataConversionBSONXML()
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
       value_builder << "Value" << BSON_ARRAY(BSON(dopamine::converterBSON::Tag_Alphabetic << "Doe^John"));
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
       value_builder << "Value" << BSON_ARRAY((long long)10);
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
       value_builder << "Value" << BSON_ARRAY((long long)6);
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

   ~TestDataConversionBSONXML()
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

BOOST_FIXTURE_TEST_CASE(ConversionBSONXML, TestDataConversionBSONXML)
{
    // Convert original BSON to XML
    dopamine::converterBSON::BSONToXML bsontoxml;
    auto xml = bsontoxml.to_ptree(bsonobject);

    // Convert XML to new BSON
    dopamine::converterBSON::XMLToBSON xmltobson;
    mongo::BSONObj const newbson = xmltobson.from_ptree(xml);

    // Compare original BSON with new
    isEqual(bsonobject, newbson);
    // Compare new BSON with original
    isEqual(newbson, bsonobject);
}


/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Converter BSON <-> XML
 */
BOOST_FIXTURE_TEST_CASE(ConversionBSONXML_String, TestDataConversionBSONXML)
{
    // Convert original BSON to XML
    dopamine::converterBSON::BSONToXML bsontoxml;
    std::string const xml = bsontoxml.to_string(bsonobject);

    // Convert XML to new BSON
    dopamine::converterBSON::XMLToBSON xmltobson;
    mongo::BSONObj const newbson = xmltobson.from_string(xml);

    // Compare original BSON with new
    isEqual(bsonobject, newbson);
    // Compare new BSON with original
    isEqual(newbson, bsonobject);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Converter XML <-> BSON
 */
struct component_name
{
    std::string _first_name;
    std::string _given_name;
    std::string _middle_name;
    std::string _prefix;
    std::string _suffix;
    std::string _tag;

    component_name(std::string const & firstname,
                   std::string const & givenname,
                   std::string const & middlename,
                   std::string const & prefix,
                   std::string const & suffix,
                   std::string const & tag):
        _first_name(firstname), _given_name(givenname),
        _middle_name(middlename), _prefix(prefix),
        _suffix(suffix), _tag(tag) {}
};

template<typename TType>
void create_dicomattribute(boost::property_tree::ptree & nativetree,
                           std::string const & dicom_tag,
                           std::string const & dicom_vr,
                           std::string const & dicom_keyword,
                           std::vector<TType> const & values,
                           std::string const & child_type = dopamine::converterBSON::Tag_Value)
{
    boost::property_tree::ptree dicomattribute;
    dicomattribute.put(dopamine::converterBSON::Attribute_Tag, dicom_tag);
    dicomattribute.put(dopamine::converterBSON::Attribute_VR, dicom_vr);
    dicomattribute.put(dopamine::converterBSON::Attribute_Keyword, dicom_keyword);

    unsigned int count = 0;
    for (auto value : values)
    {
        ++count;
        std::stringstream number;
        number << count;
        boost::property_tree::ptree tagvalue;
        tagvalue.put(dopamine::converterBSON::Attribute_Number, number.str());
        tagvalue.put_value<TType>(value);

        dicomattribute.add_child(child_type, tagvalue);
    }

    nativetree.add_child(dopamine::converterBSON::Tag_DicomAttribute, dicomattribute);
}

void create_dicomattribute_pn(boost::property_tree::ptree & nativetree,
                              std::string const & dicom_tag,
                              std::string const & dicom_keyword,
                              std::vector<component_name> const & values)
{
    boost::property_tree::ptree dicomattribute;
    dicomattribute.put(dopamine::converterBSON::Attribute_Tag, dicom_tag);
    dicomattribute.put(dopamine::converterBSON::Attribute_VR, "PN");
    dicomattribute.put(dopamine::converterBSON::Attribute_Keyword, dicom_keyword);

    unsigned int count = 0;
    for (auto name : values)
    {
        ++count;
        std::stringstream number;
        number << count;

        boost::property_tree::ptree componentname;
        boost::property_tree::ptree firstname;
        firstname.put_value<std::string>(name._first_name);
        componentname.add_child(dopamine::converterBSON::Tag_FamilyName, firstname);
        boost::property_tree::ptree givenname;
        givenname.put_value<std::string>(name._given_name);
        componentname.add_child(dopamine::converterBSON::Tag_GivenName, givenname);
        boost::property_tree::ptree middlename;
        middlename.put_value<std::string>(name._middle_name);
        componentname.add_child(dopamine::converterBSON::Tag_MiddleName, middlename);
        boost::property_tree::ptree prefix;
        prefix.put_value<std::string>(name._prefix);
        componentname.add_child(dopamine::converterBSON::Tag_NamePrefix, prefix);
        boost::property_tree::ptree suffix;
        suffix.put_value<std::string>(name._suffix);
        componentname.add_child(dopamine::converterBSON::Tag_NameSuffix, suffix);

        boost::property_tree::ptree personname;
        personname.put(dopamine::converterBSON::Attribute_Number, number.str());
        personname.add_child(name._tag, componentname);

        dicomattribute.add_child(dopamine::converterBSON::Tag_PersonName, personname);
    }

    nativetree.add_child(dopamine::converterBSON::Tag_DicomAttribute, dicomattribute);
}

void create_dicomattribute_sq(boost::property_tree::ptree & nativetree,
                              std::string const & dicom_tag,
                              std::string const & dicom_keyword,
                              std::vector<boost::property_tree::ptree> const & values)
{
    boost::property_tree::ptree dicomattributeSQ;
    dicomattributeSQ.put(dopamine::converterBSON::Attribute_Tag, dicom_tag);
    dicomattributeSQ.put(dopamine::converterBSON::Attribute_VR, "SQ");
    dicomattributeSQ.put(dopamine::converterBSON::Attribute_Keyword, dicom_keyword);

    unsigned int count = 0;
    for (auto tagitem : values)
    {
        ++count;
        std::stringstream number;
        number << count;

        boost::property_tree::ptree item;
        item.put(dopamine::converterBSON::Attribute_Number, number.str());
        item.add_child(dopamine::converterBSON::Tag_DicomAttribute, tagitem);

        dicomattributeSQ.add_child(dopamine::converterBSON::Tag_Item, item);
    }

    nativetree.add_child(dopamine::converterBSON::Tag_DicomAttribute, dicomattributeSQ);
}

struct TestDataConversionXMLBSON
{
    boost::property_tree::ptree tree;

    TestDataConversionXMLBSON()
    {
        std::vector<std::string> valuesstring = { "value01", "value02" };
        std::vector<Float64> valuesfloat64 = { 11.11, 22.22 };
        std::vector<Float32> valuesfloat32 = { 33.33, 44.44 };
        std::vector<Sint32> valuessint32 = { 111, 222 };
        std::vector<Sint16> valuessint16 = { 333, 444 };
        std::vector<Uint32> valuesuint32 = { 555, 666 };
        std::vector<Uint16> valuesuint16 = { 777, 888 };
        std::vector<std::string> valuesstringBinary = { "YXplcnR5dWlvcHFzZGZnaGprbG13eGN2Ym4xMjM0NTY" };

        boost::property_tree::ptree nativetree;

        create_dicomattribute(nativetree, "00080054", "AE", "RetrieveAE​Title", valuesstring);
        create_dicomattribute(nativetree, "00101010", "AS", "Patient​Age", valuesstring);
        create_dicomattribute(nativetree, "00209165", "AT", "Dimension​Index​Pointer", valuesstring);
        create_dicomattribute(nativetree, "00080060", "CS", "Modality", valuesstring);
        create_dicomattribute(nativetree, "00100030", "DA", "Patient​Birth​Date", valuesstring);
        create_dicomattribute(nativetree, "00101030", "DS", "Patient​Weight", valuesfloat64);
        create_dicomattribute(nativetree, "00189074", "DT", "Frame​Acquisition​Date​Time", valuesstring);
        create_dicomattribute(nativetree, "00460044", "FD", "Pupil​Size", valuesfloat64);
        create_dicomattribute(nativetree, "00089459", "FL", "Recommended​Display​Frame​Rate​InFloat", valuesfloat32);
        create_dicomattribute(nativetree, "00082122", "IS", "Stage​Number", valuessint32);
        create_dicomattribute(nativetree, "00080070", "LO", "Manufacturer", valuesstring);
        create_dicomattribute(nativetree, "001021b0", "LT", "Additional​Patient​History", valuesstring);
        create_dicomattribute(nativetree, "00282000", "OB", "ICC​Profile", valuesstringBinary, dopamine::converterBSON::Tag_InlineBinary);
        create_dicomattribute(nativetree, "00640009", "OF", "Vector​Grid​Data", valuesstringBinary, dopamine::converterBSON::Tag_InlineBinary);
        create_dicomattribute(nativetree, "00660023", "OW", "Triangle​Point​Index​List", valuesstringBinary, dopamine::converterBSON::Tag_InlineBinary);

        std::vector<component_name> valuespn =
        { component_name("Doe", "John", "Wallas", "Rev.", "Chief Executive Officer",
                         dopamine::converterBSON::Tag_Alphabetic),
          component_name("Smith", "Jane", "Scarlett", "Ms.", "Goddess",
                         dopamine::converterBSON::Tag_Alphabetic)
        };
        create_dicomattribute_pn(nativetree, "00100010", "PatientName", valuespn);

        create_dicomattribute(nativetree, "00102160", "SH", "Ethnic​Group", valuesstring);
        create_dicomattribute(nativetree, "00186020", "SL", "Reference​PixelX0", valuessint32);

        boost::property_tree::ptree value;
        value.put(dopamine::converterBSON::Attribute_Number, "1");
        value.put_value<std::string>("valueLO1");
        boost::property_tree::ptree attributeLO;
        attributeLO.put(dopamine::converterBSON::Attribute_Tag, "00100020");
        attributeLO.put(dopamine::converterBSON::Attribute_VR, "LO");
        attributeLO.put(dopamine::converterBSON::Attribute_Keyword, "PatientID");
        attributeLO.add_child(dopamine::converterBSON::Tag_Value, value);
        std::vector<boost::property_tree::ptree> valuessq = { attributeLO, attributeLO };
        create_dicomattribute_sq(nativetree, "00101002", "Other​Patient​IDs​Sequence", valuessq);

        create_dicomattribute(nativetree, "00189219", "SS", "Tag​Angle​Second​Axis", valuessint16);
        create_dicomattribute(nativetree, "00080081", "ST", "Institution​Address", valuesstring);
        create_dicomattribute(nativetree, "00080013", "TM", "Instance​Creation​Time", valuesstring);
        create_dicomattribute(nativetree, "00080016", "UI", "SOP​ClassUID", valuesstring);
        create_dicomattribute(nativetree, "00081161", "UL", "Simple​Frame​List", valuesuint32);
        // UN
        create_dicomattribute(nativetree, "00081197", "US", "Failure​Reason", valuesuint16);
        create_dicomattribute(nativetree, "00287fe0", "UT", "Pixel​Data​ProviderURL", valuesstring);

        tree.add_child(dopamine::converterBSON::Tag_NativeDicomModel, nativetree);
    }

    ~TestDataConversionXMLBSON()
    {
        // Nothing to do
    }
};

BOOST_FIXTURE_TEST_CASE(ConversionXMLBSON, TestDataConversionXMLBSON)
{
    // Convert original XML to BSON
    dopamine::converterBSON::XMLToBSON xmltobson;
    mongo::BSONObj const newbson = xmltobson.from_ptree(tree);

    // Convert BSON to new XML
    dopamine::converterBSON::BSONToXML bsontoxml;
    boost::property_tree::ptree xml = bsontoxml.to_ptree(newbson);

    // Compare original BSON with new
    // Operator == not working
    //BOOST_CHECK_EQUAL(tree == xml, true);

    std::stringstream stream_original;
    std::stringstream stream_final;
    boost::property_tree::xml_writer_settings<char> settings(' ', 4);
    boost::property_tree::write_xml(stream_original, tree, settings);
    boost::property_tree::write_xml(stream_final, tree, settings);

    BOOST_CHECK_EQUAL(stream_original.str(), stream_final.str());
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Converter XML <-> BSON
 */
BOOST_AUTO_TEST_CASE(ConversionXMLBSON_String)
{
    std::stringstream xmlstream;
    xmlstream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    xmlstream << "<NativeDicomModel>\n";
    xmlstream << "    <DicomAttribute tag=\"00080054\" vr=\"AE\" keyword=\"RetrieveAETitle\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00101010\" vr=\"AS\" keyword=\"PatientAge\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00209165\" vr=\"AT\" keyword=\"DimensionIndexPointer\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00080060\" vr=\"CS\" keyword=\"Modality\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00100030\" vr=\"DA\" keyword=\"PatientBirthDate\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00101030\" vr=\"DS\" keyword=\"PatientWeight\">\n";
    xmlstream << "        <Value number=\"1\">11.11</Value>\n";
    xmlstream << "        <Value number=\"2\">22.22</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00189074\" vr=\"DT\" keyword=\"FrameAcquisitionDateTime\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00460044\" vr=\"FD\" keyword=\"PupilSize\">\n";
    xmlstream << "        <Value number=\"1\">11.11</Value>\n";
    xmlstream << "        <Value number=\"2\">22.22</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00089459\" vr=\"FL\" keyword=\"RecommendedDisplayFrameRateInFloat\">\n";
    xmlstream << "        <Value number=\"1\">33.33000183105469</Value>\n";
    xmlstream << "        <Value number=\"2\">44.43999862670898</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00082122\" vr=\"IS\" keyword=\"StageNumber\">\n";
    xmlstream << "        <Value number=\"1\">111</Value>\n";
    xmlstream << "        <Value number=\"2\">222</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00080070\" vr=\"LO\" keyword=\"Manufacturer\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"001021b0\" vr=\"LT\" keyword=\"AdditionalPatientHistory\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00282000\" vr=\"OB\" keyword=\"ICCProfile\">\n";
    xmlstream << "        <InlineBinary>YXplcnR5dWlvcHFzZGZnaGprbG13eGN2Ym4xMjM0NTY=</InlineBinary>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00640009\" vr=\"OF\" keyword=\"VectorGridData\">\n";
    xmlstream << "        <InlineBinary>YXplcnR5dWlvcHFzZGZnaGprbG13eGN2Ym4xMjM0NTY=</InlineBinary>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00660023\" vr=\"OW\" keyword=\"TrianglePointIndexList\">\n";
    xmlstream << "        <InlineBinary>YXplcnR5dWlvcHFzZGZnaGprbG13eGN2Ym4xMjM0NTY=</InlineBinary>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00100010\" vr=\"PN\" keyword=\"PatientName\">\n";
    xmlstream << "        <PersonName number=\"1\">\n";
    xmlstream << "            <Alphabetic>\n";
    xmlstream << "                <FamilyName>Doe</FamilyName>\n";
    xmlstream << "                <GivenName>John</GivenName>\n";
    xmlstream << "                <MiddleName>Wallas</MiddleName>\n";
    xmlstream << "                <NamePrefix>Rev.</NamePrefix>\n";
    xmlstream << "                <NameSuffix>Chief Executive Officer</NameSuffix>\n";
    xmlstream << "            </Alphabetic>\n";
    xmlstream << "        </PersonName>\n";
    xmlstream << "        <PersonName number=\"2\">\n";
    xmlstream << "            <Alphabetic>\n";
    xmlstream << "                <FamilyName>Smith</FamilyName>\n";
    xmlstream << "                <GivenName>Jane</GivenName>\n";
    xmlstream << "                <MiddleName>Scarlett</MiddleName>\n";
    xmlstream << "                <NamePrefix>Ms.</NamePrefix>\n";
    xmlstream << "                <NameSuffix>Goddess</NameSuffix>\n";
    xmlstream << "            </Alphabetic>\n";
    xmlstream << "        </PersonName>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00102160\" vr=\"SH\" keyword=\"EthnicGroup\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00186020\" vr=\"SL\" keyword=\"ReferencePixelX0\">\n";
    xmlstream << "        <Value number=\"1\">111</Value>\n";
    xmlstream << "        <Value number=\"2\">222</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00101002\" vr=\"SQ\" keyword=\"OtherPatientIDsSequence\">\n";
    xmlstream << "        <Item number=\"1\">\n";
    xmlstream << "            <DicomAttribute tag=\"00100020\" vr=\"LO\" keyword=\"PatientID\">\n";
    xmlstream << "                <Value number=\"1\">valueLO1</Value>\n";
    xmlstream << "            </DicomAttribute>\n";
    xmlstream << "        </Item>\n";
    xmlstream << "        <Item number=\"2\">\n";
    xmlstream << "            <DicomAttribute tag=\"00100020\" vr=\"LO\" keyword=\"PatientID\">\n";
    xmlstream << "                <Value number=\"1\">valueLO1</Value>\n";
    xmlstream << "            </DicomAttribute>\n";
    xmlstream << "        </Item>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00189219\" vr=\"SS\" keyword=\"TagAngleSecondAxis\">\n";
    xmlstream << "        <Value number=\"1\">333</Value>\n";
    xmlstream << "        <Value number=\"2\">444</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00080081\" vr=\"ST\" keyword=\"InstitutionAddress\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00080013\" vr=\"TM\" keyword=\"InstanceCreationTime\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00080016\" vr=\"UI\" keyword=\"SOPClassUID\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00081161\" vr=\"UL\" keyword=\"SimpleFrameList\">\n";
    xmlstream << "        <Value number=\"1\">555</Value>\n";
    xmlstream << "        <Value number=\"2\">666</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00081197\" vr=\"US\" keyword=\"FailureReason\">\n";
    xmlstream << "        <Value number=\"1\">777</Value>\n";
    xmlstream << "        <Value number=\"2\">888</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "    <DicomAttribute tag=\"00287fe0\" vr=\"UT\" keyword=\"PixelDataProviderURL\">\n";
    xmlstream << "        <Value number=\"1\">value01</Value>\n";
    xmlstream << "        <Value number=\"2\">value02</Value>\n";
    xmlstream << "    </DicomAttribute>\n";
    xmlstream << "</NativeDicomModel>\n";

    // Convert original XML to BSON
    dopamine::converterBSON::XMLToBSON xmltobson;
    mongo::BSONObj const bson_ = xmltobson.from_string(xmlstream.str());

    // Convert BSON to new XML
    dopamine::converterBSON::BSONToXML bsontoxml;
    std::string const newxml = bsontoxml.to_string(bson_);

    BOOST_CHECK_EQUAL(xmlstream.str(), newxml);
}
