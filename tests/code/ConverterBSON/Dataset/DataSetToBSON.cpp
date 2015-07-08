/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleDataSetToBSON
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/test/unit_test.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "ConverterBSON/Dataset/DataSetToBSON.h"
#include "ConverterBSON/Dataset/TagMatch.h"

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::converterBSON::DataSetToBSON * datasettobson =
            new dopamine::converterBSON::DataSetToBSON();

    // Object build
    BOOST_CHECK(datasettobson != NULL);

    // Default value
    BOOST_CHECK(datasettobson->get_specific_character_set() == "");
    BOOST_CHECK(datasettobson->get_default_filter() ==
                dopamine::converterBSON::DataSetToBSON::FilterAction::INCLUDE);
    BOOST_CHECK(datasettobson->get_filters().size() == 0);

    delete datasettobson;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Getter and Setter
 */
BOOST_AUTO_TEST_CASE(GetterAndSetter)
{
    dopamine::converterBSON::DataSetToBSON datasettobson;

    // set_specific_character_set
    datasettobson.set_specific_character_set("ISO_IR 192");
    // Default value
    BOOST_CHECK(datasettobson.get_specific_character_set() == "ISO_IR 192");

    // set_default_filter
    datasettobson.set_default_filter(
                dopamine::converterBSON::DataSetToBSON::FilterAction::EXCLUDE);
    // check value
    BOOST_CHECK(datasettobson.get_default_filter() ==
                dopamine::converterBSON::DataSetToBSON::FilterAction::EXCLUDE);

    // Set Filter
    std::vector<dopamine::converterBSON::DataSetToBSON::Filter> filters;
    filters.push_back(std::make_pair(
              dopamine::converterBSON::TagMatch::New(DCM_PatientName),
              dopamine::converterBSON::DataSetToBSON::FilterAction::INCLUDE));

    datasettobson.set_filters(filters);

    // check value
    BOOST_CHECK_EQUAL(datasettobson.get_filters().size(), 1);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR AE
 */
BOOST_AUTO_TEST_CASE(ConversionAE)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_RetrieveAETitle, "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080054");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "AE");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value01");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR AS
 */
BOOST_AUTO_TEST_CASE(ConversionAS)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_PatientAge, "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00101010");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "AS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value01");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR AT
 */
BOOST_AUTO_TEST_CASE(ConversionAT)
{
    DcmDataset* dataset = new DcmDataset();
    DcmElement * element = NULL;
    OFCondition condition = dataset->insertEmptyElement(
                DCM_DimensionIndexPointer);
    BOOST_REQUIRE(condition == EC_Normal);
    condition = dataset->findAndGetElement(DCM_DimensionIndexPointer,
                                           element);
    BOOST_REQUIRE(condition == EC_Normal);
    BOOST_REQUIRE(element != NULL);
    std::vector<Uint16> vectoruint16 = { DCM_PatientName.getGroup() ,
                                         DCM_PatientName.getElement() };
    element->putUint16Array(&vectoruint16[0], 1);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement elementbson = it.next();
    mongo::BSONObj object = elementbson.Obj();
    BOOST_CHECK_EQUAL(elementbson.fieldName(), "00209165");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "AT");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(),
                      "(0010,0010)");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR CS
 */
BOOST_AUTO_TEST_CASE(ConversionCS)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_Modality, "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080060");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "CS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value01");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR DA
 */
BOOST_AUTO_TEST_CASE(ConversionDA)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_PatientBirthDate, "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00100030");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "DA");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value01");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR DS
 */
BOOST_AUTO_TEST_CASE(ConversionDS)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_PatientWeight, "61.5\\62.5");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00101030");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "DS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Double(), 61.5);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Double(), 62.5);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR DT
 */
BOOST_AUTO_TEST_CASE(ConversionDT)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_FrameAcquisitionDateTime,
                                       "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00189074");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "DT");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value01");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR FD
 */
BOOST_AUTO_TEST_CASE(ConversionFD)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->insertEmptyElement(DCM_PupilSize);
    DcmElement * element = NULL;
    OFCondition condition = dataset->findAndGetElement(DCM_PupilSize,
                                                       element);
    BOOST_REQUIRE(condition == EC_Normal);
    BOOST_REQUIRE(element != NULL);
    std::vector<Float64> vectorfloat64 = {42.5, 43.6};
    element->putFloat64Array(&vectorfloat64[0], 2);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement elementbson = it.next();
    mongo::BSONObj object = elementbson.Obj();
    BOOST_CHECK_EQUAL(elementbson.fieldName(), "00460044");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "FD");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Double(), 42.5);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Double(), 43.6);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR FL
 */
BOOST_AUTO_TEST_CASE(ConversionFL)
{
    DcmDataset* dataset = new DcmDataset();
    DcmElement * element = NULL;
    OFCondition condition = dataset->insertEmptyElement(
                DCM_RecommendedDisplayFrameRateInFloat);
    BOOST_REQUIRE(condition == EC_Normal);
    condition = dataset->findAndGetElement(DCM_RecommendedDisplayFrameRateInFloat,
                                           element);
    BOOST_REQUIRE(condition == EC_Normal);
    BOOST_REQUIRE(element != NULL);
    std::vector<Float32> vectorfloat32 = {42.5, 43.6};
    element->putFloat32Array(&vectorfloat32[0], 2);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement elementbson = it.next();
    mongo::BSONObj object = elementbson.Obj();
    BOOST_CHECK_EQUAL(elementbson.fieldName(), "00089459");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "FL");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_CLOSE(object.getField("Value").Array()[0].Double(), 42.5, 0.001);
    BOOST_CHECK_CLOSE(object.getField("Value").Array()[1].Double(), 43.6, 0.001);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR IS
 */
BOOST_AUTO_TEST_CASE(ConversionIS)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_StageNumber, "12\\13");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00082122");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "IS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 12);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Int(), 13);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR LO
 */
BOOST_AUTO_TEST_CASE(ConversionLO)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_Manufacturer, "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080070");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "LO");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value01");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR LT
 */
BOOST_AUTO_TEST_CASE(ConversionLT)
{
    DcmDataset* dataset = new DcmDataset();
    // Be carefull: putAndInsertOFStringArray for LT add only 1 value !!!
    dataset->putAndInsertOFStringArray(DCM_AdditionalPatientHistory,
                                       "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "001021b0");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "LT");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(),
            "value01\\value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}


/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR OB
 */
BOOST_AUTO_TEST_CASE(ConversionOB)
{
    std::string const value = "azertyuiopqsdfghjklmwxcvbn123456";
    DcmDataset* dataset = new DcmDataset();
    OFCondition condition = dataset->putAndInsertUint8Array(
                DCM_ICCProfile,
                reinterpret_cast<Uint8 const *>(value.c_str()),
                32);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00282000");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "OB");
    BOOST_CHECK(object.getField("InlineBinary").type() ==
                mongo::BSONType::BinData);

    int size=0;
    char const * begin = object.getField("InlineBinary").binDataClean(size);
    BOOST_CHECK_EQUAL(std::string(begin, size), value);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR OF
 */
BOOST_AUTO_TEST_CASE(ConversionOF)
{
    std::string const value = "azertyuiopqsdfghjklmwxcvbn123456";
    DcmDataset* dataset = new DcmDataset();
    DcmElement * element = NULL;
    OFCondition condition = dataset->insertEmptyElement(DCM_VectorGridData);
    BOOST_REQUIRE(condition == EC_Normal);
    condition = dataset->findAndGetElement(DCM_VectorGridData, element);
    BOOST_REQUIRE(condition == EC_Normal);
    BOOST_REQUIRE(element != NULL);
    element->putFloat32Array(reinterpret_cast<Float32 const *>(value.c_str()),
                             8);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement elementbson = it.next();
    mongo::BSONObj object = elementbson.Obj();
    BOOST_CHECK_EQUAL(elementbson.fieldName(), "00640009");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "OF");
    BOOST_CHECK(object.getField("InlineBinary").type() ==
                mongo::BSONType::BinData);

    int size=0;
    char const * begin = object.getField("InlineBinary").binDataClean(size);
    BOOST_CHECK_EQUAL(std::string(begin, size), value);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR OW
 */
BOOST_AUTO_TEST_CASE(ConversionOW)
{
    std::string const value = "azertyuiopqsdfghjklmwxcvbn123456";
    DcmDataset* dataset = new DcmDataset();
    DcmElement * element = NULL;
    OFCondition condition = dataset->insertEmptyElement(
                DCM_TrianglePointIndexList);
    BOOST_REQUIRE(condition == EC_Normal);
    condition = dataset->findAndGetElement(DCM_TrianglePointIndexList,
                                           element);
    BOOST_REQUIRE(condition == EC_Normal);
    BOOST_REQUIRE(element != NULL);
    condition = element->putUint16Array(
                reinterpret_cast<Uint16 const *>(value.c_str()), 16);
    BOOST_REQUIRE(condition == EC_Normal);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement elementbson = it.next();
    mongo::BSONObj object = elementbson.Obj();
    BOOST_CHECK_EQUAL(elementbson.fieldName(), "00660023");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "OW");
    BOOST_CHECK(object.getField("InlineBinary").type() ==
                mongo::BSONType::BinData);

    int size=0;
    char const * begin = object.getField("InlineBinary").binDataClean(size);
    BOOST_CHECK_EQUAL(std::string(begin, size), value);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR PN
 */
BOOST_AUTO_TEST_CASE(ConversionPN)
{
    std::string const name1 = "Doe^John^Wallas^Rev.^Chief Executive Officer";
    std::string const name2 = "Smith^Jane^Scarlett^Ms.^Goddess";
    std::stringstream names;
    names << name1 << "\\" << name2;

    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_PatientName, names.str().c_str());

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00100010");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "PN");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);

    mongo::BSONObj component_name = object.getField("Value").Array()[0].Obj();
    BOOST_CHECK_EQUAL(component_name.getField("Alphabetic").String(), name1);
    component_name = object.getField("Value").Array()[1].Obj();
    BOOST_CHECK_EQUAL(component_name.getField("Alphabetic").String(), name2);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR SH
 */
BOOST_AUTO_TEST_CASE(ConversionSH)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_EthnicGroup, "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00102160");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SH");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value01");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR SL
 */
BOOST_AUTO_TEST_CASE(ConversionSL)
{
    DcmDataset* dataset = new DcmDataset();
    DcmElement * element = NULL;
    dataset->insertEmptyElement(DCM_ReferencePixelX0);
    dataset->findAndGetElement(DCM_ReferencePixelX0, element);
    std::vector<Sint32> vectorsint32 = {10, 11};
    element->putSint32Array(&vectorsint32[0], 2);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement elementbson = it.next();
    mongo::BSONObj object = elementbson.Obj();
    BOOST_CHECK_EQUAL(elementbson.fieldName(), "00186020");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SL");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 10);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Int(), 11);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR SQ
 */
BOOST_AUTO_TEST_CASE(ConversionSQ)
{
    DcmDataset* dataset = new DcmDataset();
    DcmItem* item = new DcmItem(DCM_OtherPatientIDsSequence);
    item->putAndInsertOFStringArray(DCM_PatientID, "123");
    dataset->insertSequenceItem(DCM_OtherPatientIDsSequence, item);
    item = new DcmItem(DCM_OtherPatientIDsSequence);
    item->putAndInsertOFStringArray(DCM_PatientID, "123");
    dataset->insertSequenceItem(DCM_OtherPatientIDsSequence, item, 1);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement elementbson = it.next();
    mongo::BSONObj object = elementbson.Obj();
    BOOST_CHECK_EQUAL(elementbson.fieldName(), "00101002");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SQ");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);

    mongo::BSONObj object_item = object.getField("Value").Array()[0].Obj();
    mongo::BSONObj::iterator it_item = object_item.begin();
    mongo::BSONElement elementbson_item = it_item.next();
    mongo::BSONObj object_subitem = elementbson_item.Obj();
    BOOST_CHECK_EQUAL(elementbson_item.fieldName(), "00100020");
    BOOST_CHECK_EQUAL(object_subitem.getField("vr").String(), "LO");
    BOOST_CHECK_EQUAL(object_subitem.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(object_subitem["Value"].Array()[0].String(), "123");

    object_item = object.getField("Value").Array()[1].Obj();
    it_item = object_item.begin();
    elementbson_item = it_item.next();
    object_subitem = elementbson_item.Obj();
    BOOST_CHECK_EQUAL(elementbson_item.fieldName(), "00100020");
    BOOST_CHECK_EQUAL(object_subitem.getField("vr").String(), "LO");
    BOOST_CHECK_EQUAL(object_subitem.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(object_subitem["Value"].Array()[0].String(), "123");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR SS
 */
BOOST_AUTO_TEST_CASE(ConversionSS)
{
    DcmDataset* dataset = new DcmDataset();
    DcmElement * element = NULL;
    dataset->insertEmptyElement(DCM_TagAngleSecondAxis);
    dataset->findAndGetElement(DCM_TagAngleSecondAxis, element);
    std::vector<Sint16> vectorsint16 = {10, 11};
    element->putSint16Array(&vectorsint16[0], 2);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement elementbson = it.next();
    mongo::BSONObj object = elementbson.Obj();
    BOOST_CHECK_EQUAL(elementbson.fieldName(), "00189219");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 10);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Int(), 11);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR ST
 */
BOOST_AUTO_TEST_CASE(ConversionST)
{
    DcmDataset* dataset = new DcmDataset();
    // Be carefull: putAndInsertOFStringArray for ST add only 1 value !!!
    dataset->putAndInsertOFStringArray(DCM_InstitutionAddress,
                                       "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080081");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "ST");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(),
                      "value01\\value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR TM
 */
BOOST_AUTO_TEST_CASE(ConversionTM)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationTime,
                                       "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080013");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "TM");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value01");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR UI
 */
BOOST_AUTO_TEST_CASE(ConversionUI)
{
    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_SOPClassUID, "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080016");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "UI");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value01");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR UL
 */
BOOST_AUTO_TEST_CASE(ConversionUL)
{
    DcmDataset* dataset = new DcmDataset();
    DcmElement * element = NULL;
    dataset->insertEmptyElement(DCM_SimpleFrameList);
    dataset->findAndGetElement(DCM_SimpleFrameList, element);
    std::vector<Uint32> vectoruint32 = {10, 11};
    element->putUint32Array(&vectoruint32[0], 2);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement elementbson = it.next();
    mongo::BSONObj object = elementbson.Obj();
    BOOST_CHECK_EQUAL(elementbson.fieldName(), "00081161");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "UL");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 10);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Int(), 11);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR UN
 */
BOOST_AUTO_TEST_CASE(ConversionUN)
{
    // TODO
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR US
 */
BOOST_AUTO_TEST_CASE(ConversionUS)
{
    DcmDataset* dataset = new DcmDataset();
    DcmElement * element = NULL;
    dataset->insertEmptyElement(DCM_FailureReason);
    dataset->findAndGetElement(DCM_FailureReason, element);
    std::vector<Uint16> vectoruint16 = {10, 11};
    element->putUint16Array(&vectoruint16[0], 2);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement elementbson = it.next();
    mongo::BSONObj object = elementbson.Obj();
    BOOST_CHECK_EQUAL(elementbson.fieldName(), "00081197");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "US");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 10);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Int(), 11);

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR UT
 */
BOOST_AUTO_TEST_CASE(ConversionUT)
{
    DcmDataset* dataset = new DcmDataset();
    // Be carefull: putAndInsertOFStringArray for UT add only 1 value !!!
    dataset->putAndInsertOFStringArray(DCM_PixelDataProviderURL,
                                       "value01\\value02");

    dopamine::converterBSON::DataSetToBSON datasettobson;
    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element = it.next();
    mongo::BSONObj object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00287fe0");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "UT");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(),
                      "value01\\value02");

    BOOST_CHECK_EQUAL(it.more(), false);

    delete dataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Filter
 */
BOOST_AUTO_TEST_CASE(Filter_Exclude_all)
{
    dopamine::converterBSON::DataSetToBSON datasettobson;

    // set_default_filter
    datasettobson.set_default_filter(
            dopamine::converterBSON::DataSetToBSON::FilterAction::EXCLUDE);

    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_Modality, "value01\\value02");
    dataset->putAndInsertOFStringArray(DCM_PatientBirthDate,
                                       "value01\\value02");

    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    BOOST_CHECK_EQUAL(query_dataset.isEmpty(), true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Filter
 */
BOOST_AUTO_TEST_CASE(Filter_Exclude_field)
{
    dopamine::converterBSON::DataSetToBSON datasettobson;

    // set_default_filter
    datasettobson.set_default_filter(
            dopamine::converterBSON::DataSetToBSON::FilterAction::INCLUDE);
    std::vector<dopamine::converterBSON::DataSetToBSON::Filter> filters;
    filters.push_back(std::make_pair(
              dopamine::converterBSON::TagMatch::New(DCM_PatientBirthDate),
              dopamine::converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
    datasettobson.set_filters(filters);

    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_Modality, "value01\\value02");
    dataset->putAndInsertOFStringArray(DCM_PatientBirthDate, "value01\\value02");

    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    BOOST_CHECK_EQUAL(query_dataset.hasField("00080060"), true);
    BOOST_CHECK_EQUAL(query_dataset.hasField("00100030"), false);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Set Specific character set with Dataset element
 */
BOOST_AUTO_TEST_CASE(Set_Specific_Character_Set)
{
    dopamine::converterBSON::DataSetToBSON datasettobson;

    // set_default_filter
    datasettobson.set_default_filter(
                dopamine::converterBSON::DataSetToBSON::FilterAction::INCLUDE);

    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_SpecificCharacterSet, "ISO_IR 192");
    dataset->putAndInsertOFStringArray(DCM_Modality, "value01\\value02");

    BOOST_CHECK_EQUAL(datasettobson.get_specific_character_set(), "");

    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    BOOST_CHECK_EQUAL(query_dataset.hasField("00080060"), true);
    BOOST_CHECK_EQUAL(datasettobson.get_specific_character_set(), "ISO_IR 192");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Ignore Group length attribute
 */
BOOST_AUTO_TEST_CASE(Group_length_tag)
{
    dopamine::converterBSON::DataSetToBSON datasettobson;

    // set_default_filter
    datasettobson.set_default_filter(
                dopamine::converterBSON::DataSetToBSON::FilterAction::INCLUDE);

    DcmDataset* dataset = new DcmDataset();
    dataset->putAndInsertOFStringArray(DCM_Modality, "value01\\value02");
    DcmElement * element = NULL;
    dataset->insertEmptyElement(DCM_FileMetaInformationGroupLength);
    dataset->findAndGetElement(DCM_FileMetaInformationGroupLength, element);
    std::vector<Uint32> vectoruint32 = {10};
    element->putUint32Array(&vectoruint32[0], 1);

    mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

    BOOST_CHECK_EQUAL(query_dataset.hasField("00080060"), true);
    BOOST_CHECK_EQUAL(query_dataset.hasField("00020000"), false);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Specific character set
 */
struct fileAndResult
{
    std::string _filename;
    std::string _alphabetic;
    std::string _ideographic;
    std::string _phonetic;

    fileAndResult(std::string filename, std::string alphabetic,
                  std::string ideographic = "", std::string phonetic = ""):
        _filename(filename), _alphabetic(alphabetic),
        _ideographic(ideographic), _phonetic(phonetic) {}
};

BOOST_AUTO_TEST_CASE(Specific_charsetset)
{
    dopamine::converterBSON::DataSetToBSON datasettobson;

    // set_default_filter
    datasettobson.set_default_filter(
                dopamine::converterBSON::DataSetToBSON::FilterAction::INCLUDE);

    // Get data directory
    std::string datadirectory(getenv("DOPAMINE_TEST_DATA"));
    datadirectory = datadirectory + "/charsettests/";

    std::stringstream arabicname;
    arabicname << "\xd9\x82\xd8\xa8\xd8\xa7\xd9\x86\xd9\x8a" << "^"
               << "\xd9\x84\xd9\x86\xd8\xb2\xd8\xa7\xd8\xb1";

    std::stringstream germanname;
    germanname << "\xc3\x84neas^R\xc3\xbc" << "diger";

    std::stringstream greekname;
    greekname << "\xce\x94\xce\xb9\xce\xbf\xce\xbd\xcf"
              << "\x85\xcf\x83\xce\xb9\xce\xbf\xcf\x82";

    std::stringstream h31name;
    h31name << "\xe3\x82\x84\xe3\x81\xbe\xe3\x81\xa0" << "^"
            << "\xe3\x81\x9f\xe3\x82\x8d\xe3\x81\x86";

    std::stringstream h32name_alphabetic;
    h32name_alphabetic << "\xef\xbe\x94\xef\xbe\x8f\xef\xbe\x80\xef\xbe\x9e"
                       << "^" << "\xef\xbe\x80\xef\xbe\x9b\xef\xbd\xb3";

    std::stringstream h32name_phonetic;
    h32name_phonetic << "\xe3\x82\x84\xe3\x81\xbe\xe3\x81\xa0" << "^"
                     << "\xe3\x81\x9f\xe3\x82\x8d\xe3\x81\x86";

    std::stringstream hbrwname;
    hbrwname << "\xd7\xa9\xd7\xa8\xd7\x95\xd7\x9f" << "^"
             << "\xd7\x93\xd7\x91\xd7\x95\xd7\xa8\xd7\x94";

    std::stringstream i2name_ideographic;
    i2name_ideographic << "\x1b\x24\x29\x43\xe6\xb4\xaa" << "^"
                       << "\x1b\x24\x29\x43\xe5\x90\x89\xe6\xb4\x9e";

    std::stringstream i2name_phonetic;
    i2name_phonetic << "\x1b\x24\x29\x43\xed\x99\x8d" << "^"
                    << "\x1b\x24\x29\x43\xea\xb8\xb8\xeb\x8f\x99";

    std::stringstream russianname;
    russianname << "\xd0\x9b\xd1\x8e\xd0\xba" << "ce\xd0\xbc\xd0\xb1yp\xd0\xb3";

    std::vector<fileAndResult> files_to_test =
    {
        fileAndResult("SCSARAB", arabicname.str()),
        fileAndResult("SCSFREN", "Buc^J\xc3\xa9r\xc3\xb4me"),
        fileAndResult("SCSGERM", germanname.str()),
        fileAndResult("SCSGREEK", greekname.str()),
        fileAndResult("SCSH31", "Yamada^Tarou",
                      "\xe5\xb1\xb1\xe7\x94\xb0^\xe5\xa4\xaa\xe9\x83\x8e",
                      h31name.str()),
        fileAndResult("SCSH32", h32name_alphabetic.str(),
                      "\xe5\xb1\xb1\xe7\x94\xb0^\xe5\xa4\xaa\xe9\x83\x8e",
                      h32name_phonetic.str()),
        fileAndResult("SCSHBRW", hbrwname.str()),
        fileAndResult("SCSI2", "Hong^Gildong", i2name_ideographic.str(),
                      i2name_phonetic.str()),
        fileAndResult("SCSRUSS", russianname.str()),
        fileAndResult("SCSX1", "Wang^XiaoDong",
                      "\xe7\x8e\x8b^\xe5\xb0\x8f\xe6\x9d\xb1"),
        fileAndResult("SCSX2", "Wang^XiaoDong",
                      "\xe7\x8e\x8b^\xe5\xb0\x8f\xe4\xb8\x9c")
    };

    for (fileAndResult file_to_test : files_to_test)
    {
        std::string file = datadirectory + file_to_test._filename;
        DcmFileFormat fileformat;
        OFCondition result = fileformat.loadFile(file.c_str());
        if (result.bad())
        {
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(result.text()));
        }
        DcmDataset* dataset = fileformat.getAndRemoveDataset();

        OFString ofpatientname;
        result = dataset->findAndGetOFString(DCM_PatientName, ofpatientname);
        BOOST_REQUIRE(result.good());

        std::vector<std::string> name_components;
        std::string strtemp(ofpatientname.c_str());
        boost::split(name_components, strtemp,
                     boost::is_any_of("="));

        mongo::BSONObj const query_dataset = datasettobson.from_dataset(dataset);

        BOOST_REQUIRE(query_dataset.hasField("00100010"));
        mongo::BSONElement patientname = query_dataset.getField("00100010");
        BOOST_REQUIRE(patientname.type() == mongo::BSONType::Object);
        BOOST_REQUIRE(patientname.Obj().hasField("Value"));
        mongo::BSONElement values = patientname.Obj().getField("Value");
        BOOST_REQUIRE(values.type() == mongo::BSONType::Array);
        BOOST_REQUIRE_EQUAL(values.Array().size(), name_components.size());

        if (name_components.size() > 0)
        {
            BOOST_REQUIRE(values.Array()[0].Obj().hasField("Alphabetic"));
            BOOST_CHECK_EQUAL(values.Array()[0].Obj()["Alphabetic"].String(),
                              file_to_test._alphabetic);
        }
        if (name_components.size() > 1)
        {
            BOOST_REQUIRE(values.Array()[1].Obj().hasField("Ideographic"));
            BOOST_CHECK_EQUAL(values.Array()[1].Obj()["Ideographic"].String(),
                              file_to_test._ideographic);
        }
        if (name_components.size() > 2)
        {
            BOOST_REQUIRE(values.Array()[2].Obj().hasField("Phonetic"));
            BOOST_CHECK_EQUAL(values.Array()[2].Obj()["Phonetic"].String(),
                              file_to_test._phonetic);
        }
        if (name_components.size() > 3)
        {
            BOOST_FAIL("too many values for patient's name");
        }

        delete dataset;
    }
}

/*************************** TEST Error *********************************/
/**
 * Error test case: set_specific_character_set => bad value
 */
BOOST_AUTO_TEST_CASE(Specific_charset_badvalue)
{
    dopamine::converterBSON::DataSetToBSON datasettobson;

    // set_specific_character_set
    BOOST_REQUIRE_THROW(datasettobson.set_specific_character_set("badvalue"),
                        dopamine::ExceptionPACS);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Throw Unhandled VR
 */
BOOST_AUTO_TEST_CASE(Unhandled_VR)
{
    DcmDataset* dataset = new DcmDataset();
    DcmAttributeTag* element =
            new DcmAttributeTag(DcmTag(0x9998,0x9998,DcmVR(EVR_na)));
    std::string test = "(0008,0020)\(0008,0030)";
    OFCondition cond = element->putString(test.c_str());
    dataset->insert(element);

    dopamine::converterBSON::DataSetToBSON datasettobson;
    BOOST_REQUIRE_THROW(datasettobson.from_dataset(dataset);,
                        std::runtime_error);

    delete dataset;
}
