/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleBSONToDataSet
#include <boost/test/unit_test.hpp>

#include <mongo/bson/bson.h>
#include <mongo/db/json.h>

#include "ConverterBSON/Dataset/BSONToDataSet.h"

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::converterBSON::BSONToDataSet * bsontodataset =
            new dopamine::converterBSON::BSONToDataSet();

    // Object build
    BOOST_CHECK_EQUAL(bsontodataset != NULL, true);
    
    // Default value
    BOOST_CHECK_EQUAL(bsontodataset->get_specific_character_set() == "", true);

    delete bsontodataset;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Getter and Setter
 */
BOOST_AUTO_TEST_CASE(GetterAndSetter)
{
    dopamine::converterBSON::BSONToDataSet bsontodataset;

    // set_specific_character_set
    bsontodataset.set_specific_character_set("ISO_IR 192");
    
    // Default value
    BOOST_CHECK_EQUAL(bsontodataset.get_specific_character_set() == "ISO_IR 192", true);
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
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_RetrieveAETitle, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_RetrieveAETitle, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR AS
 */
BOOST_AUTO_TEST_CASE(ConversionAS)
{
    // Create BSON with AS tag
    std::string const tag = "00101010";
    std::string const vr = "AS";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_PatientAge, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_PatientAge, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR AT
 */
BOOST_AUTO_TEST_CASE(ConversionAT)
{
    // Create BSON with AT tag
    std::string const tag = "00209165";
    std::string const vr = "AT";
    mongo::BSONArray const values = BSON_ARRAY("(0010,0010)" << "(0010,0020)");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_DimensionIndexPointer, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "(0010,0010)");
    condition = dataset.findAndGetOFString(DCM_DimensionIndexPointer, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "(0010,0020)");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR CS
 */
BOOST_AUTO_TEST_CASE(ConversionCS)
{
    // Create BSON with CS tag
    std::string const tag = "00080060";
    std::string const vr = "CS";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_Modality, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_Modality, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR DA
 */
BOOST_AUTO_TEST_CASE(ConversionDA)
{
    // Create BSON with DA tag
    std::string const tag = "00100030";
    std::string const vr = "DA";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_PatientBirthDate, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_PatientBirthDate, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
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
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_PatientWeight, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "11.11");
    condition = dataset.findAndGetOFString(DCM_PatientWeight, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "22.22");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR DT
 */
BOOST_AUTO_TEST_CASE(ConversionDT)
{
    // Create BSON with DT tag
    std::string const tag = "00189074";
    std::string const vr = "DT";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_FrameAcquisitionDateTime, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_FrameAcquisitionDateTime, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR FD
 */
BOOST_AUTO_TEST_CASE(ConversionFD)
{
    // Create BSON with FD tag
    std::string const tag = "00460044";
    std::string const vr = "FD";
    mongo::BSONArray const values = BSON_ARRAY(44.4 << 55.5);
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    Float64 value;
    OFCondition condition = dataset.findAndGetFloat64(DCM_PupilSize, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 44.4);
    condition = dataset.findAndGetFloat64(DCM_PupilSize, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 55.5);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR FL
 */
BOOST_AUTO_TEST_CASE(ConversionFL)
{
    // Create BSON with FL tag
    std::string const tag = "00089459";
    std::string const vr = "FL";
    mongo::BSONArray const values = BSON_ARRAY(77.7 << 88.8);
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    Float32 value;
    OFCondition condition = dataset.findAndGetFloat32(DCM_RecommendedDisplayFrameRateInFloat, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_CLOSE(value, 77.7, 0.001);
    condition = dataset.findAndGetFloat32(DCM_RecommendedDisplayFrameRateInFloat, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_CLOSE(value, 88.8, 0.001);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR IS
 */
BOOST_AUTO_TEST_CASE(ConversionIS)
{
    // Create BSON with IS tag
    std::string const tag = "00082122";
    std::string const vr = "IS";
    mongo::BSONArray const values = BSON_ARRAY(111 << 222);
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_StageNumber, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "111");
    condition = dataset.findAndGetOFString(DCM_StageNumber, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "222");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR LO
 */
BOOST_AUTO_TEST_CASE(ConversionLO)
{
    // Create BSON with LO tag
    std::string const tag = "00080070";
    std::string const vr = "LO";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_Manufacturer, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_Manufacturer, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR LT
 */
BOOST_AUTO_TEST_CASE(ConversionLT)
{
    // Create BSON with LT tag
    std::string const tag = "001021b0";
    std::string const vr = "LT";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_AdditionalPatientHistory, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    // Be carefull: putAndInsertOFStringArray for LT add only 1 value !!!
    BOOST_CHECK_EQUAL(value.c_str(), "value1\\value2");
    /*condition = dataset.findAndGetOFString(DCM_AdditionalPatientHistory, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1\\value2");*/
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
    std::string const value = "azertyuiopqsdfghjklmwxcvbn123456";
    mongo::BSONObjBuilder binary_data_builder;
    binary_data_builder.appendBinData("data", value.size(),
                                      mongo::BinDataGeneral, (void*)(value.c_str()));
    mongo::BSONObj object =
            BSON(tag <<
                 BSON("vr" << vr <<
                      "InlineBinary" << binary_data_builder.obj().getField("data")));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    std::stringstream stream;
    for (unsigned long i = 0; i < 32; ++i)
    {
        Uint8 values;
        OFCondition condition = dataset.findAndGetUint8(DCM_ICCProfile, values, i);
        BOOST_CHECK_EQUAL(condition == EC_Normal, true);
        stream << values;
    }
    BOOST_CHECK_EQUAL(value, stream.str());
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR OF
 */
BOOST_AUTO_TEST_CASE(ConversionOF)
{
    // Create BSON with OF tag
    std::string const tag = "00640009";
    std::string const vr = "OF";
    std::string const value = "azertyuiopqsdfghjklmwxcvbn123456";
    mongo::BSONObjBuilder binary_data_builder;
    binary_data_builder.appendBinData("data", value.size(),
                                      mongo::BinDataGeneral, (void*)(value.c_str()));
    mongo::BSONObj object =
            BSON(tag <<
                 BSON("vr" << vr <<
                      "InlineBinary" << binary_data_builder.obj().getField("data")));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    DcmElement* element = NULL;
    OFCondition condition = dataset.findAndGetElement(DCM_VectorGridData, element);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);

    void* begin(NULL);
    condition = element->getFloat32Array(*reinterpret_cast<Float32**>(&begin));
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);

    // Check result
    std::string result(reinterpret_cast<char*>(begin), element->getLength());
    BOOST_CHECK_EQUAL(value, result);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR OW
 */
BOOST_AUTO_TEST_CASE(ConversionOW)
{
    // Create BSON with OW tag
    std::string const tag = "00660023";
    std::string const vr = "OW";
    std::string const value = "azertyuiopqsdfghjklmwxcvbn123456";
    mongo::BSONObjBuilder binary_data_builder;
    binary_data_builder.appendBinData("data", value.size(),
                                      mongo::BinDataGeneral, (void*)(value.c_str()));
    mongo::BSONObj object =
            BSON(tag <<
                 BSON("vr" << vr <<
                      "InlineBinary" << binary_data_builder.obj().getField("data")));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    DcmElement* element = NULL;
    OFCondition condition = dataset.findAndGetElement(DCM_TrianglePointIndexList, element);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);

    void* begin(NULL);
    condition = element->getUint16Array(*reinterpret_cast<Uint16**>(&begin));
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);

    // Check result
    std::string result(reinterpret_cast<char*>(begin), element->getLength());
    BOOST_CHECK_EQUAL(value, result);
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
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_PatientName, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "Doe^John^Wallas^Rev.^Chief Executive Officer");
    condition = dataset.findAndGetOFString(DCM_PatientName, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "Smith^Jane^Scarlett^Ms.^Goddess");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR SH
 */
BOOST_AUTO_TEST_CASE(ConversionSH)
{
    // Create BSON with SH tag
    std::string const tag = "00102160";
    std::string const vr = "SH";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_EthnicGroup, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_EthnicGroup, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR SL
 */
BOOST_AUTO_TEST_CASE(ConversionSL)
{
    // Create BSON with SL tag
    std::string const tag = "00186020";
    std::string const vr = "SL";
    mongo::BSONArray const values = BSON_ARRAY(1001 << 2002);
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    Sint32 value;
    OFCondition condition = dataset.findAndGetSint32(DCM_ReferencePixelX0, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 1001);
    condition = dataset.findAndGetSint32(DCM_ReferencePixelX0, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 2002);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR SQ
 */
BOOST_AUTO_TEST_CASE(ConversionSQ)
{
    // TODO

    // Create 2 BSON with LO and CS tags
    std::string const tagLO = "00100020";
    std::string const vrLO = "LO";
    mongo::BSONArray const valuesLO = BSON_ARRAY("value1" << "value2");
    std::string const tagCS = "00100022";
    std::string const vrCS = "CS";
    mongo::BSONArray const valuesCS = BSON_ARRAY("valueCS1" << "valueCS2");
    mongo::BSONObj object_1 = BSON(tagLO << BSON("vr" << vrLO << "Value" << valuesLO)
                                << tagCS << BSON("vr" << vrCS << "Value" << valuesCS));

    mongo::BSONObj object_2 = BSON(tagLO << BSON("vr" << vrLO << "Value" << valuesLO)
                                << tagCS << BSON("vr" << vrCS << "Value" << valuesCS));

    // Create BSON with SQ tag
    std::string const tag = "00101002";
    std::string const vr = "SQ";
    mongo::BSONArray const values =
            BSON_ARRAY(object_1 << object_2);
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    DcmItem* item = NULL;
    OFCondition condition = dataset.findAndGetSequenceItem(DCM_OtherPatientIDsSequence, item, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(item != NULL, true);

    OFString value;
    condition = item->findAndGetOFString(DCM_PatientID, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = item->findAndGetOFString(DCM_PatientID, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");

    condition = item->findAndGetOFString(DCM_TypeOfPatientID, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "valueCS1");
    condition = item->findAndGetOFString(DCM_TypeOfPatientID, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "valueCS2");

    item = NULL;
    condition = dataset.findAndGetSequenceItem(DCM_OtherPatientIDsSequence, item, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(item != NULL, true);

    condition = item->findAndGetOFString(DCM_PatientID, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = item->findAndGetOFString(DCM_PatientID, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");

    condition = item->findAndGetOFString(DCM_TypeOfPatientID, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "valueCS1");
    condition = item->findAndGetOFString(DCM_TypeOfPatientID, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "valueCS2");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR SS
 */
BOOST_AUTO_TEST_CASE(ConversionSS)
{
    // Create BSON with SS tag
    std::string const tag = "00189219";
    std::string const vr = "SS";
    mongo::BSONArray const values = BSON_ARRAY(555 << 666);
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    Sint16 value;
    OFCondition condition = dataset.findAndGetSint16(DCM_TagAngleSecondAxis, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 555);
    condition = dataset.findAndGetSint16(DCM_TagAngleSecondAxis, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 666);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR ST
 */
BOOST_AUTO_TEST_CASE(ConversionST)
{
    // Create BSON with ST tag
    std::string const tag = "00080081";
    std::string const vr = "ST";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_InstitutionAddress, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    // Be carefull: putAndInsertOFStringArray for ST add only 1 value !!!
    BOOST_CHECK_EQUAL(value.c_str(), "value1\\value2");
    /*condition = dataset.findAndGetOFString(DCM_InstitutionAddress, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");*/
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR TM
 */
BOOST_AUTO_TEST_CASE(ConversionTM)
{
    // Create BSON with TM tag
    std::string const tag = "00080013";
    std::string const vr = "TM";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_InstanceCreationTime, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_InstanceCreationTime, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR UI
 */
BOOST_AUTO_TEST_CASE(ConversionUI)
{
    // Create BSON with UI tag
    std::string const tag = "00080016";
    std::string const vr = "UI";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_SOPClassUID, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_SOPClassUID, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR UL
 */
BOOST_AUTO_TEST_CASE(ConversionUL)
{
    // Create BSON with UL tag
    std::string const tag = "00081161";
    std::string const vr = "UL";
    mongo::BSONArray const values = BSON_ARRAY(111 << 333);
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    Uint32 value;
    OFCondition condition = dataset.findAndGetUint32(DCM_SimpleFrameList, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 111);
    condition = dataset.findAndGetUint32(DCM_SimpleFrameList, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 333);
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
    // Create BSON with US tag
    std::string const tag = "00081197";
    std::string const vr = "US";
    mongo::BSONArray const values = BSON_ARRAY(77 << 88);
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    Uint16 value;
    OFCondition condition = dataset.findAndGetUint16(DCM_FailureReason, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 77);
    condition = dataset.findAndGetUint16(DCM_FailureReason, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 88);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR UT
 */
BOOST_AUTO_TEST_CASE(ConversionUT)
{
    // Create BSON with UT tag
    std::string const tag = "00287fe0";
    std::string const vr = "UT";
    mongo::BSONArray const values = BSON_ARRAY("value1" << "value2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    // Check result
    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_PixelDataProviderURL, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    // Be carefull: putAndInsertOFStringArray for UT add only 1 value !!!
    BOOST_CHECK_EQUAL(value.c_str(), "value1\\value2");
    /*condition = dataset.findAndGetOFString(DCM_PixelDataProviderURL, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");*/
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Set Specific character set with BSON element
 */
BOOST_AUTO_TEST_CASE(Set_Specific_Character_Set)
{
    // Create BSON with UT tag
    std::string const tag = "00080005";
    std::string const vr = "CS";
    mongo::BSONArray const values = BSON_ARRAY("ISO_IR 192");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    BOOST_CHECK_EQUAL(bsontodataset.get_specific_character_set(), "");
    DcmDataset dataset = bsontodataset.to_dataset(object);
    BOOST_CHECK_EQUAL(bsontodataset.get_specific_character_set(), "ISO_IR 192");

    OFString value;
    OFCondition condition = dataset.findAndGetOFString(DCM_SpecificCharacterSet, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "ISO_IR 192");
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Ignore non DICOM tag
 */
BOOST_AUTO_TEST_CASE(Not_DICOM_Tag)
{
    // Create BSON with CS tag
    std::string const tag = "X0080060";
    std::string const vr = "CS";
    mongo::BSONArray const values = BSON_ARRAY("valueCS1" << "valueCS2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset.to_dataset(object);

    BOOST_CHECK_EQUAL(dataset.getElement(0) == NULL, true);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: set_specific_character_set => bad value
 */
BOOST_AUTO_TEST_CASE(Specific_charset_badvalue)
{
    dopamine::converterBSON::BSONToDataSet bsontodataset;

    // set_specific_character_set
    BOOST_REQUIRE_THROW(bsontodataset.set_specific_character_set("badvalue"),
                        std::runtime_error);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: set_specific_character_set => multi-valued
 */
BOOST_AUTO_TEST_CASE(Specific_charset_multiValue)
{
    // Create BSON with UT tag
    std::string const tag = "00080005";
    std::string const vr = "CS";
    mongo::BSONArray const values = BSON_ARRAY("ISO_IR 192" << "GB18030");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    BOOST_CHECK_EQUAL(bsontodataset.get_specific_character_set(), "");
    BOOST_REQUIRE_THROW(bsontodataset.to_dataset(object), std::runtime_error);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Throw Unhandled VR
 */
struct TestDataKO02
{
    mongo::BSONObj bsonobject;

    TestDataKO02()
    {
        mongo::BSONObjBuilder bsonobjectbuilder;

        mongo::BSONObjBuilder value_builder;
        value_builder << "vr" << "an";
        value_builder << "Value" << BSON_ARRAY("temp");
        bsonobjectbuilder << "99989998" << value_builder.obj();

        bsonobject = bsonobjectbuilder.obj();
    }

    ~TestDataKO02()
    {
        // Nothing to do
    }
};

BOOST_FIXTURE_TEST_CASE(Unhandled_VR, TestDataKO02)
{
    dopamine::converterBSON::BSONToDataSet bsontodataset;
    BOOST_REQUIRE_THROW(bsontodataset.to_dataset(bsonobject),
                        std::runtime_error);
}
