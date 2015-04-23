/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleDataSetToBSON
#include <boost/test/unit_test.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "ConverterBSON/Dataset/DataSetToBSON.h"
#include "ConverterBSON/Dataset/TagMatch.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
 struct TestDataOK01
{
    dopamine::DataSetToBSON * datasettobson;
 
    TestDataOK01()
    {
        datasettobson = new dopamine::DataSetToBSON();
    }
 
    ~TestDataOK01()
    {
        delete datasettobson;
    }
};

BOOST_FIXTURE_TEST_CASE(Constructor, TestDataOK01)
{
    // Object build
    BOOST_CHECK_EQUAL(datasettobson != NULL, true);
    
    // Default value
    BOOST_CHECK_EQUAL(datasettobson->get_specific_character_set() == "", true);
    BOOST_CHECK_EQUAL(datasettobson->get_default_filter() == 
                      dopamine::DataSetToBSON::FilterAction::INCLUDE, true);
    BOOST_CHECK_EQUAL(datasettobson->get_filters().size() == 0, true);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Getter and Setter
 */
BOOST_FIXTURE_TEST_CASE(GetterAndSetter, TestDataOK01)
{
    // set_specific_character_set
    datasettobson->set_specific_character_set("ISO_IR 192");
    // Default value
    BOOST_CHECK_EQUAL(datasettobson->get_specific_character_set() == "ISO_IR 192", true);

    // set_default_filter
    datasettobson->set_default_filter(dopamine::DataSetToBSON::FilterAction::EXCLUDE);
    // check value
    BOOST_CHECK_EQUAL(datasettobson->get_default_filter() == 
                      dopamine::DataSetToBSON::FilterAction::EXCLUDE, true);

    // Set Filter
    std::vector<dopamine::DataSetToBSON::Filter> filters;
    filters.push_back(std::make_pair(dopamine::TagMatch::New(DCM_PatientName),
                      dopamine::DataSetToBSON::FilterAction::INCLUDE));
                      
    datasettobson->set_filters(filters);
    
    std::vector<dopamine::DataSetToBSON::Filter> const getfilters =
        datasettobson->get_filters();
    // check value
    BOOST_CHECK_EQUAL(getfilters.size(), 1);
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Operator () single values
 */
 struct TestDataOperatorBracketSingleValue
{
    dopamine::DataSetToBSON * datasettobson;
    DcmDataset* dataset;
 
    TestDataOperatorBracketSingleValue()
    {
        datasettobson = new dopamine::DataSetToBSON();
        dataset = new DcmDataset();
        dataset->putAndInsertOFStringArray(DCM_RetrieveAETitle, "test_AE");                         // insert AE
        dataset->putAndInsertOFStringArray(DCM_PatientAge, "test_AS");                              // insert AS
        dataset->putAndInsertOFStringArray(DCM_Modality, "value1");                                 // insert CS
        dataset->putAndInsertOFStringArray(DCM_PatientBirthDate, "01/01/2001");                     // insert DA
        dataset->putAndInsertOFStringArray(DCM_PatientWeight, "60.5");                              // insert DS
        dataset->putAndInsertOFStringArray(DCM_FrameAcquisitionDateTime, "01/01/2001 09:09:09");    // insert DT
        dataset->putAndInsertFloat64(DCM_PupilSize, 42.5);                                          // insert FD
        dataset->putAndInsertFloat32(DCM_RecommendedDisplayFrameRateInFloat, 15.2);                 // insert FL
        dataset->putAndInsertOFStringArray(DCM_StageNumber, "12");                                  // insert IS
        dataset->putAndInsertOFStringArray(DCM_Manufacturer, "MyManufacturer");                     // insert LO
        dataset->putAndInsertOFStringArray(DCM_AdditionalPatientHistory, "test_valueLT");           // insert LT
        dataset->putAndInsertOFStringArray(DCM_PatientName, "Doe^John");                            // insert PN
        dataset->putAndInsertOFStringArray(DCM_EthnicGroup, "test_valueSH");                        // insert SH
        dataset->putAndInsertSint32(DCM_ReferencePixelX0, 10);                                      // insert SL
        dataset->putAndInsertSint16(DCM_TagAngleSecondAxis, 10);                                    // insert SS
        dataset->putAndInsertOFStringArray(DCM_InstitutionAddress, "MyAddress");                    // insert ST
        dataset->putAndInsertOFStringArray(DCM_InstanceCreationTime, "08:08:08");                   // insert TM
        dataset->putAndInsertOFStringArray(DCM_SOPClassUID, "1.2.3.4.5.6");                         // insert UI
        dataset->putAndInsertUint32(DCM_SimpleFrameList, 11);                                       // insert UL
        dataset->putAndInsertUint16(DCM_FailureReason, 5);                                          // insert US
        dataset->putAndInsertOFStringArray(DCM_PixelDataProviderURL, "test_valueUT");               // insert UT
        
        // insert SQ
        DcmItem* item = new DcmItem(DCM_OtherPatientIDsSequence);
        item->putAndInsertOFStringArray(DCM_PatientID, "123");
        dataset->insertSequenceItem(DCM_OtherPatientIDsSequence, item);
    }
 
    ~TestDataOperatorBracketSingleValue()
    {
        delete datasettobson;
        delete dataset;
    }
};

BOOST_FIXTURE_TEST_CASE(OperatorBracketSingleValue, TestDataOperatorBracketSingleValue)
{
    mongo::BSONObjBuilder query_builder;
    (*datasettobson)(dataset, query_builder);
    
    mongo::BSONObj const query_dataset = query_builder.obj();
    
    mongo::BSONObj::iterator it = query_dataset.begin();
    
    mongo::BSONElement element;
    mongo::BSONObj object;
    
    // Testing TM
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080013");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "TM");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "08:08:08");
    
    // Testing UI
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080016");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "UI");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "1.2.3.4.5.6");
    
    // Testing AE
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080054");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "AE");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "test_AE");
    
    // Testing CS
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080060");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "CS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value1");
    
    // Testing LO
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080070");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "LO");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "MyManufacturer");
    
    // Testing ST
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080081");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "ST");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "MyAddress");
    
    // Testing UL
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00081161");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "UL");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 11);
    
    // Testing US
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00081197");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "US");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 5);
    
    // Testing IS
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00082122");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "IS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 12);
    
    // Testing FL
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00089459");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "FL");
    BOOST_CHECK_EQUAL((Float32)object.getField("Value").Array()[0].Double(), (Float32)15.2);
    
    // Testing PN
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00100010");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "PN");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "Doe^John");
    
    // Testing DA
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00100030");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "DA");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "01/01/2001");
    
    // Testing SQ
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00101002");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SQ");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Obj().isValid(), true);

    // Testing AS
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00101010");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "AS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "test_AS");
    
    // Testing DS
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00101030");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "DS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Double(), 60.5);
    
    // Testing SH
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00102160");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SH");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "test_valueSH");
    
    // Testing LT
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "001021b0");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "LT");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "test_valueLT");
    
    // Testing SL
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00186020");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SL");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 10);
    
    // Testing DT
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00189074");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "DT");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "01/01/2001 09:09:09");
    
    // Testing SS
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00189219");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 10);
    
    // Testing UT
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00287fe0");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "UT");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "test_valueUT");
    
    // Testing FD
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00460044");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "FD");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Double(), 42.5);
}


/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: Operator () Multiple values
 */
 struct TestDataOperatorBracketMultipleValues
{
    dopamine::DataSetToBSON * datasettobson;
    DcmDataset* dataset;

    TestDataOperatorBracketMultipleValues()
    {
        DcmElement * element = NULL;

        datasettobson = new dopamine::DataSetToBSON();
        dataset = new DcmDataset();
        dataset->putAndInsertOFStringArray(DCM_RetrieveAETitle, "test_AE\\test_AE2\\test_AE3");             // insert AE
        dataset->putAndInsertOFStringArray(DCM_PatientAge, "test_AS\\test_AS2\\test_AS3");                  // insert AS
        dataset->putAndInsertOFStringArray(DCM_Modality, "value1\\value2\\value3");                         // insert CS
        dataset->putAndInsertOFStringArray(DCM_PatientBirthDate, "01/01/2001\\02/02/2002\\03/03/2003");     // insert DA
        dataset->putAndInsertOFStringArray(DCM_PatientWeight, "61.5\\62.5\\63.5");                          // insert DS
        dataset->putAndInsertOFStringArray(DCM_FrameAcquisitionDateTime,
                                           "01/01/2001 09:09:09\\02/02/2002 09:09:09\\03/03/2003 09:09:09");// insert DT

        dataset->insertEmptyElement(DCM_PupilSize);
        dataset->findAndGetElement(DCM_PupilSize, element);
        std::vector<Float64> vectorfloat64 = {42.5, 43.6, 44.7};
        element->putFloat64Array(&vectorfloat64[0], 3);                                                     // insert FD

        dataset->insertEmptyElement(DCM_RecommendedDisplayFrameRateInFloat);
        dataset->findAndGetElement(DCM_RecommendedDisplayFrameRateInFloat, element);
        std::vector<Float32> vectorfloat32 = {42.5, 43.6, 44.7};
        element->putFloat32Array(&vectorfloat32[0], 3);                                                     // insert FL

        dataset->putAndInsertOFStringArray(DCM_StageNumber, "12\\13\\14");                                  // insert IS
        dataset->putAndInsertOFStringArray(DCM_Manufacturer, "MyManufacturer\\value2\\value3");             // insert LO
        dataset->putAndInsertOFStringArray(DCM_AdditionalPatientHistory, "test_valueLT\\value2\\value3");   // insert LT
        dataset->putAndInsertOFStringArray(DCM_PatientName, "Doe^John\\Doe^Jane\\Doe^Jim");                 // insert PN
        dataset->putAndInsertOFStringArray(DCM_EthnicGroup, "test_valueSH\\value2\\value3");                // insert SH

        dataset->insertEmptyElement(DCM_ReferencePixelX0);
        dataset->findAndGetElement(DCM_ReferencePixelX0, element);
        std::vector<Sint32> vectorsint32 = {10, 11, 12};
        element->putSint32Array(&vectorsint32[0], 3);                                                       // insert SL

        dataset->insertEmptyElement(DCM_TagAngleSecondAxis);
        dataset->findAndGetElement(DCM_TagAngleSecondAxis, element);
        std::vector<Sint16> vectorsint16 = {10, 11, 12};
        element->putSint16Array(&vectorsint16[0], 3);                                                       // insert SS

        dataset->putAndInsertOFStringArray(DCM_InstitutionAddress, "MyAddress\\value2\\value3");            // insert ST
        dataset->putAndInsertOFStringArray(DCM_InstanceCreationTime, "08:08:08\\09:08:08\\10:08:08");       // insert TM
        dataset->putAndInsertOFStringArray(DCM_SOPClassUID, "1.2.3.4.5.6\\1.2.3.4.5.7\\1.2.3.4.5.8");       // insert UI

        dataset->insertEmptyElement(DCM_SimpleFrameList);
        dataset->findAndGetElement(DCM_SimpleFrameList, element);
        std::vector<Uint32> vectoruint32 = {10, 11, 12};
        element->putUint32Array(&vectoruint32[0], 3);                                                       // insert UL

        dataset->insertEmptyElement(DCM_FailureReason);
        dataset->findAndGetElement(DCM_FailureReason, element);
        std::vector<Uint16> vectoruint16 = {10, 11, 12};
        element->putUint16Array(&vectoruint16[0], 3);                                                       // insert US

        dataset->putAndInsertOFStringArray(DCM_PixelDataProviderURL, "test_valueUT\\value2\\value3");       // insert UT

        // insert SQ
        DcmItem* item = new DcmItem(DCM_OtherPatientIDsSequence);
        item->putAndInsertOFStringArray(DCM_PatientID, "123");
        dataset->insertSequenceItem(DCM_OtherPatientIDsSequence, item);
        item = new DcmItem(DCM_OtherPatientIDsSequence);
        item->putAndInsertOFStringArray(DCM_PatientID, "123");
        dataset->insertSequenceItem(DCM_OtherPatientIDsSequence, item, 1);
    }

    ~TestDataOperatorBracketMultipleValues()
    {
        delete datasettobson;
        delete dataset;
    }
};

BOOST_FIXTURE_TEST_CASE(OperatorBracketMultipleValues, TestDataOperatorBracketMultipleValues)
{
    mongo::BSONObjBuilder query_builder;
    (*datasettobson)(dataset, query_builder);

    mongo::BSONObj const query_dataset = query_builder.obj();

    mongo::BSONObj::iterator it = query_dataset.begin();

    mongo::BSONElement element;
    mongo::BSONObj object;

    // Testing TM
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080013");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "TM");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "08:08:08");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "09:08:08");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "10:08:08");

    // Testing UI
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080016");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "UI");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "1.2.3.4.5.6");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "1.2.3.4.5.7");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "1.2.3.4.5.8");

    // Testing AE
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080054");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "AE");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "test_AE");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "test_AE2");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "test_AE3");

    // Testing CS
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080060");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "CS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "value1");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value2");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "value3");

    // Testing LO
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080070");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "LO");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "MyManufacturer");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value2");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "value3");

    // Testing ST
    // Be carefull: putAndInsertOFStringArray for ST add only 1 value !!!
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080081");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "ST");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "MyAddress\\value2\\value3");
    //BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value2");
    //BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "value3");

    // Testing UL
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00081161");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "UL");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 10);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Int(), 11);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].Int(), 12);

    // Testing US
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00081197");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "US");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 10);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Int(), 11);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].Int(), 12);

    // Testing IS
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00082122");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "IS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 12);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Int(), 13);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].Int(), 14);

    // Testing FL
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00089459");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "FL");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_CLOSE(object.getField("Value").Array()[0].Double(), 42.5, 0.001);
    BOOST_CHECK_CLOSE(object.getField("Value").Array()[1].Double(), 43.6, 0.001);
    BOOST_CHECK_CLOSE(object.getField("Value").Array()[2].Double(), 44.7, 0.001);

    // Testing PN
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00100010");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "PN");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "Doe^John");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "Doe^Jane");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "Doe^Jim");

    // Testing DA
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00100030");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "DA");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "01/01/2001");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "02/02/2002");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "03/03/2003");

    // Testing SQ
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00101002");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SQ");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 2);
    mongo::BSONObj subobject = object.getField("Value").Array()[0].Obj().getField("00100020").Obj();
    BOOST_CHECK_EQUAL(subobject.getField("vr").String(), "LO");
    BOOST_CHECK_EQUAL(subobject.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(subobject.getField("Value").Array()[0].String(), "123");
    subobject = object.getField("Value").Array()[1].Obj().getField("00100020").Obj();
    BOOST_CHECK_EQUAL(subobject.getField("vr").String(), "LO");
    BOOST_CHECK_EQUAL(subobject.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(subobject.getField("Value").Array()[0].String(), "123");

    // Testing AS
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00101010");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "AS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "test_AS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "test_AS2");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "test_AS3");

    // Testing DS
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00101030");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "DS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Double(), 61.5);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Double(), 62.5);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].Double(), 63.5);

    // Testing SH
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00102160");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SH");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "test_valueSH");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value2");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "value3");

    // Testing LT
    // Be carefull: putAndInsertOFStringArray for LT add only 1 value !!!
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "001021b0");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "LT");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "test_valueLT\\value2\\value3");
    //BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value2");
    //BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "value3");

    // Testing SL
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00186020");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SL");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 10);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Int(), 11);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].Int(), 12);

    // Testing DT
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00189074");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "DT");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "01/01/2001 09:09:09");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "02/02/2002 09:09:09");
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "03/03/2003 09:09:09");

    // Testing SS
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00189219");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "SS");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].Int(), 10);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].Int(), 11);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].Int(), 12);

    // Testing UT
    // Be carefull: putAndInsertOFStringArray for UT add only 1 value !!!
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00287fe0");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "UT");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "test_valueUT\\value2\\value3");
    //BOOST_CHECK_EQUAL(object.getField("Value").Array()[1].String(), "value2");
    //BOOST_CHECK_EQUAL(object.getField("Value").Array()[2].String(), "value3");

    // Testing FD
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00460044");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "FD");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 3);
    BOOST_CHECK_CLOSE(object.getField("Value").Array()[0].Double(), 42.5, 0.001);
    BOOST_CHECK_CLOSE(object.getField("Value").Array()[1].Double(), 43.6, 0.001);
    BOOST_CHECK_CLOSE(object.getField("Value").Array()[2].Double(), 44.7, 0.001);
}

/*************************** TEST OK 05 *******************************/
/**
 * Nominal test case: Matching Tag
 */
 struct TestDataOK05
{
    dopamine::DataSetToBSON * datasettobson;
    DcmDataset* dataset;
 
    TestDataOK05()
    {
        datasettobson = new dopamine::DataSetToBSON();
        dataset = new DcmDataset();
        dataset->putAndInsertOFStringArray(DCM_Modality, "value1");     
        dataset->putAndInsertOFStringArray(DCM_PatientName, "Doe^John");
    }
 
    ~TestDataOK05()
    {
        delete datasettobson;
        delete dataset;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_05, TestDataOK05)
{
    datasettobson->get_filters().push_back(
        std::make_pair(dopamine::TagMatch::New(DCM_PatientName),
                       dopamine::DataSetToBSON::FilterAction::INCLUDE));
    datasettobson->get_filters().push_back(
        std::make_pair(dopamine::TagMatch::New(DCM_Modality),
                       dopamine::DataSetToBSON::FilterAction::EXCLUDE));
                           
    mongo::BSONObjBuilder query_builder;
    (*datasettobson)(dataset, query_builder);
    
    mongo::BSONObj const query_dataset = query_builder.obj();
    
    mongo::BSONObj::iterator it = query_dataset.begin();
    
    mongo::BSONElement element;
    mongo::BSONObj object;
    
    // Testing PN
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00100010");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "PN");
    BOOST_CHECK_EQUAL(object.getField("Value").Array().size(), 1);
    BOOST_CHECK_EQUAL(object.getField("Value").Array()[0].String(), "Doe^John");
    
    BOOST_CHECK_EQUAL(it.more(), false);
}

/*************************** TEST OK 06 *******************************/
/**
 * Nominal test case: Insert empty value
 */
 struct TestDataOK06
{
    dopamine::DataSetToBSON * datasettobson;
    DcmDataset* dataset;
 
    TestDataOK06()
    {
        datasettobson = new dopamine::DataSetToBSON();
        dataset = new DcmDataset();
        dataset->putAndInsertOFStringArray(DCM_Modality, "");
    }
 
    ~TestDataOK06()
    {
        delete datasettobson;
        delete dataset;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_06, TestDataOK06)
{
    mongo::BSONObjBuilder query_builder;
    (*datasettobson)(dataset, query_builder);
    
    mongo::BSONObj const query_dataset = query_builder.obj();
    
    mongo::BSONObj::iterator it = query_dataset.begin();
    
    mongo::BSONElement element;
    mongo::BSONObj object;
    
    // Testing TM
    element = it.next();
    object = element.Obj();
    BOOST_CHECK_EQUAL(element.fieldName(), "00080060");
    BOOST_CHECK_EQUAL(object.getField("vr").String(), "CS");
    BOOST_CHECK_EQUAL(object.getField("Value").isNull(), true);
}

/*************************** TEST OK 07 *******************************/
/**
 * Nominal test case: EVR depending on context (EVR_ox, EVR_xs, EVR_lt)
 * WARNING: NOT IMPLEMENTED
 */
 struct TestDataOK07
{
    dopamine::DataSetToBSON * datasettobson;
    DcmDataset* dataset;
 
    TestDataOK07()
    {
        datasettobson = new dopamine::DataSetToBSON();
        dataset = new DcmDataset();
        
        /*DcmElement* element = NULL;
        
        // Insert US or SS
        element = new DcmUnsignedShort(DcmTag(0x9998,0x9998,DcmVR(EVR_xs)));
        element->putUint16(5);
        dataset->insert(element);
        
        // Insert US, SS or OW
        element = new DcmOtherByteOtherWord(DcmTag(0x9998,0x9999,DcmVR(EVR_lt)));
        Uint8 * temp;
        element->createUint8Array(8, temp);
        dataset->insert(element);
        
        // Insert OB or OW
        element = new DcmOtherByteOtherWord(DcmTag(0x9998,0x999a,DcmVR(EVR_ox)));
        element->createUint8Array(8, temp);
        dataset->insert(element);*/
    }
 
    ~TestDataOK07()
    {
        delete datasettobson;
        delete dataset;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_07, TestDataOK07)
{
    /*mongo::BSONObjBuilder query_builder;
    (*datasettobson)(dataset, query_builder);
    
    mongo::BSONObj const query_dataset = query_builder.obj();
    
    mongo::BSONObj::iterator it = query_dataset.begin();
    
    mongo::BSONElement element;
    std::vector<mongo::BSONElement> array;*/
    
    // Testing US or SS
    
    // Testing US, SS or OW
    
    // Testing OB or OW
}

/*************************** TEST OK 08 *******************************/
/**
 * Nominal test case: EVR OB, OF and OW
 * WARNING: NOT IMPLEMENTED
 */
 struct TestDataOK08
{
    dopamine::DataSetToBSON * datasettobson;
    DcmDataset* dataset;
 
    TestDataOK08()
    {
        datasettobson = new dopamine::DataSetToBSON();
        dataset = new DcmDataset();
        
        // Insert OB
        
        // Insert OW
        
        // Insert OF
    }
 
    ~TestDataOK08()
    {
        delete datasettobson;
        delete dataset;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_08, TestDataOK08)
{
    /*mongo::BSONObjBuilder query_builder;
    (*datasettobson)(dataset, query_builder);
    
    mongo::BSONObj const query_dataset = query_builder.obj();
    
    mongo::BSONObj::iterator it = query_dataset.begin();
    
    mongo::BSONElement element;
    std::vector<mongo::BSONElement> array;*/
    
    // Testing OB
    
    // Testing OW
    
    // Testing OF
}
 
/*************************** TEST OK XX *******************************/
/**
 * Nominal test case: EVR UN
 * WARNING: NOT IMPLEMENTED
 */

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: set_specific_character_set => bad value
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_01, TestDataOK01)
{
    // set_specific_character_set
    BOOST_REQUIRE_THROW(datasettobson->set_specific_character_set("badvalue"),
                        std::runtime_error);
}

/*************************** TEST KO 02 *******************************/
/**
 * Error test case: set_specific_character_set => multi-valued
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_02, TestDataOK01)
{
    // set_specific_character_set
    BOOST_REQUIRE_THROW(datasettobson->set_specific_character_set("ISO_IR 192\\GB18030"),
                        std::runtime_error);
}

/*************************** TEST KO 03 *******************************/
/**
 * Error test case: Throw Unhandled VR
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_03, TestDataOK06)
{
    DcmAttributeTag* element = new DcmAttributeTag(DcmTag(0x9998,0x9998,DcmVR(EVR_na)));
    std::string test = "(0008,0020)\(0008,0030)";
    OFCondition cond = element->putString(test.c_str());
    dataset->insert(element);
    
    mongo::BSONObjBuilder query_builder;
    
    BOOST_REQUIRE_THROW((*datasettobson)(dataset, query_builder);,
                        std::runtime_error);
}
