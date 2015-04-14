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

#include "ConverterBSON/BSONToDataSet.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
 struct TestDataOK01
{
    dopamine::BSONToDataSet * bsontodataset;
 
    TestDataOK01()
    {
        bsontodataset = new dopamine::BSONToDataSet();
    }
 
    ~TestDataOK01()
    {
        delete bsontodataset;
    }
};

BOOST_FIXTURE_TEST_CASE(Constructor, TestDataOK01)
{
    // Object build
    BOOST_CHECK_EQUAL(bsontodataset != NULL, true);
    
    // Default value
    BOOST_CHECK_EQUAL(bsontodataset->get_specific_character_set() == "", true);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Getter and Setter
 */
BOOST_FIXTURE_TEST_CASE(GetterAndSetter, TestDataOK01)
{
    // set_specific_character_set
    bsontodataset->set_specific_character_set("ISO_IR 192");
    
    // Default value
    BOOST_CHECK_EQUAL(bsontodataset->get_specific_character_set() == "ISO_IR 192", true);
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Operator () Single Value
 */
 struct TestDataOperatorBracketSingleValue
{
    mongo::BSONObj bsonobject;
 
    TestDataOperatorBracketSingleValue()
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
        value_builder << "Value" << BSON_ARRAY("Doe^John");
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
 
    ~TestDataOperatorBracketSingleValue()
    {
        // Nothing to do
    }
};

BOOST_FIXTURE_TEST_CASE(OperatorBracketSingleValue, TestDataOperatorBracketSingleValue)
{
    dopamine::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset(bsonobject);

    // Testing AE
    {
    OFString value;
    dataset.findAndGetOFString(DCM_RetrieveAETitle, value);
    BOOST_CHECK_EQUAL(value.c_str(), "test_AE");
    }
    
    // Testing AS
    {
    OFString value;
    dataset.findAndGetOFString(DCM_PatientAge, value);
    BOOST_CHECK_EQUAL(value.c_str(), "test_AS");
    }
    
    // Testing CS
    {
    OFString value;
    dataset.findAndGetOFString(DCM_Modality, value);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    }
    
    // Testing DA
    {
    OFString value;
    dataset.findAndGetOFString(DCM_PatientBirthDate, value);
    BOOST_CHECK_EQUAL(value.c_str(), "01/01/2001");
    }
    
    // Testing DS
    {
    Float64 value;
    dataset.findAndGetFloat64(DCM_PatientWeight, value);
    BOOST_CHECK_EQUAL(value, (Float64)60.5);
    }
    
    // Testing DT
    {
    OFString value;
    dataset.findAndGetOFString(DCM_FrameAcquisitionDateTime, value);
    BOOST_CHECK_EQUAL(value.c_str(), "01/01/2001 09:09:09");
    }
    
    // Testing FD
    {
    Float64 value;
    dataset.findAndGetFloat64(DCM_PupilSize, value);
    BOOST_CHECK_EQUAL(value, (Float64)42.5);
    }
    
    // Testing FL
    {
    Float32 value;
    dataset.findAndGetFloat32(DCM_RecommendedDisplayFrameRateInFloat, value);
    BOOST_CHECK_EQUAL(value, (Float32)15.2);
    }
    
    // Testing IS
    {
    Sint32 value;
    dataset.findAndGetSint32(DCM_StageNumber, value);
    BOOST_CHECK_EQUAL(value, (Sint32)12);
    }
    
    // Testing LO
    {
    OFString value;
    dataset.findAndGetOFString(DCM_Manufacturer, value);
    BOOST_CHECK_EQUAL(value.c_str(), "MyManufacturer");
    }
    
    // Testing LT
    {
    OFString value;
    dataset.findAndGetOFString(DCM_AdditionalPatientHistory, value);
    BOOST_CHECK_EQUAL(value.c_str(), "test_valueLT");
    }
    
    // Testing PN
    {
    OFString value;
    dataset.findAndGetOFString(DCM_PatientName, value);
    BOOST_CHECK_EQUAL(value.c_str(), "Doe^John");
    }
    
    // Testing SH
    {
    OFString value;
    dataset.findAndGetOFString(DCM_EthnicGroup, value);
    BOOST_CHECK_EQUAL(value.c_str(), "test_valueSH");
    }
    
    // Testing SL
    {
    Sint32 value;
    dataset.findAndGetSint32(DCM_ReferencePixelX0, value);
    BOOST_CHECK_EQUAL(value, (Sint32)10);
    }
    
    // Testing SS
    {
    Sint16 value;
    dataset.findAndGetSint16(DCM_TagAngleSecondAxis, value);
    BOOST_CHECK_EQUAL(value, (Sint16)11);
    }
    
    // Testing ST
    {
    OFString value;
    dataset.findAndGetOFString(DCM_InstitutionAddress, value);
    BOOST_CHECK_EQUAL(value.c_str(), "MyAddress");
    }
    
    // Testing TM
    {
    OFString value;
    dataset.findAndGetOFString(DCM_InstanceCreationTime, value);
    BOOST_CHECK_EQUAL(value.c_str(), "08:08:08");
    }
    
    // Testing UI
    {
    OFString value;
    dataset.findAndGetOFString(DCM_SOPClassUID, value);
    BOOST_CHECK_EQUAL(value.c_str(), "1.2.3.4.5.6");
    }
    
    // Testing UL
    {
    Uint32 value;
    dataset.findAndGetUint32(DCM_SimpleFrameList, value);
    BOOST_CHECK_EQUAL(value, (Uint32)6);
    }
    
    // Testing US
    {
    Uint16 value;
    dataset.findAndGetUint16(DCM_FailureReason, value);
    BOOST_CHECK_EQUAL(value, (Uint16)5);
    }
    
    // Testing UT
    {
    OFString value;
    dataset.findAndGetOFString(DCM_PixelDataProviderURL, value);
    BOOST_CHECK_EQUAL(value.c_str(), "test_valueUT");
    }

    // Testing SQ
    {
    DcmSequenceOfItems * sequence = NULL;
    dataset.findAndGetSequence(DCM_OtherPatientIDsSequence, sequence);
    BOOST_CHECK_EQUAL(sequence != NULL, true);
    DcmItem * item = sequence->getItem(0);
    BOOST_CHECK_EQUAL(item != NULL, true);
    OFString value;
    item->findAndGetOFString(DCM_PatientID, value);
    BOOST_CHECK_EQUAL(value.c_str(), "123");
    }
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: Insert empty value
 */
 struct TestDataOK04
{
     mongo::BSONObj bsonobject;

     TestDataOK04()
     {
         mongo::BSONObjBuilder bsonobjectbuilder;

         // Insert CS
         {
         mongo::BSONObjBuilder value_builder;
         value_builder << "vr" << "CS";
         mongo::BSONArrayBuilder array_builder;
         array_builder.appendNull();
         value_builder << "Value" << array_builder.arr();
         bsonobjectbuilder << "00080060" << value_builder.obj();
         }

         bsonobject = bsonobjectbuilder.obj();
    }
 
    ~TestDataOK04()
    {
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_04, TestDataOK04)
{
    dopamine::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset(bsonobject);

    // Testing CS
    {
    OFString value;
    dataset.findAndGetOFString(DCM_Modality, value);
    BOOST_CHECK_EQUAL(value.c_str(), "");
    }
}

/*************************** TEST OK 05 *******************************/
/**
 * Nominal test case: Sequence (SQ) Element
 */
 struct TestDataOK05
{
     mongo::BSONObj bsonobject_1_2;
     mongo::BSONObj bsonobject_2_1;

    TestDataOK05()
    {
        mongo::BSONObjBuilder bsonobjectbuilder;

        // Insert SQ 1 item 2 fields
        {
        mongo::BSONObjBuilder subvalue_builder;
        subvalue_builder << "vr" << "LO";
        subvalue_builder << "Value" << BSON_ARRAY("MyManufacturer");

        mongo::BSONObjBuilder subvalue_builder2;
        subvalue_builder2 << "vr" << "CS";
        subvalue_builder2 << "Value" << BSON_ARRAY("value1");

        mongo::BSONObjBuilder value_builder;
        value_builder << "vr" << "SQ";
        value_builder << "Value" << BSON_ARRAY(BSON("00080070" << subvalue_builder.obj() <<
                                                    "00080060" << subvalue_builder2.obj()));
        bsonobjectbuilder << "00101002" << value_builder.obj();
        }

        bsonobject_1_2 = bsonobjectbuilder.obj();

        mongo::BSONObjBuilder bsonobjectbuilder_2_1;

        // Insert SQ 2 item 1 fields
        {
        mongo::BSONObjBuilder subvalue_builder;
        subvalue_builder << "vr" << "LO";
        subvalue_builder << "Value" << BSON_ARRAY("MyManufacturer");

        mongo::BSONObjBuilder subvalue_builder2;
        subvalue_builder2 << "vr" << "LO";
        subvalue_builder2 << "Value" << BSON_ARRAY("MyManufacturer2");

        mongo::BSONObjBuilder value_builder;
        value_builder << "vr" << "SQ";
        value_builder << "Value" << BSON_ARRAY(BSON("00080070" << subvalue_builder.obj()) <<
                                               BSON("00080070" << subvalue_builder2.obj()));
        bsonobjectbuilder_2_1 << "00101002" << value_builder.obj();
        }

        bsonobject_2_1 = bsonobjectbuilder_2_1.obj();
    }
 
    ~TestDataOK05()
    {
        // Nothing to do
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_05, TestDataOK05)
{
    dopamine::BSONToDataSet bsontodataset;

    /***************************************************************************/
    DcmDataset dataset = bsontodataset(bsonobject_1_2);

    // Testing SQ
    BOOST_CHECK_EQUAL(dataset.tagExists(DCM_OtherPatientIDsSequence), true);
    
    DcmItem* item = NULL;
    dataset.findOrCreateSequenceItem(DCM_OtherPatientIDsSequence, item);

    // Testing CS
    {
    OFString value;
    item->findAndGetOFString(DCM_Modality, value);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    }
    
    // Testing LO
    {
    OFString value;
    item->findAndGetOFString(DCM_Manufacturer, value);
    BOOST_CHECK_EQUAL(value.c_str(), "MyManufacturer");
    }

    /***************************************************************************/
    dataset = bsontodataset(bsonobject_2_1);

    // Testing SQ
    BOOST_CHECK_EQUAL(dataset.tagExists(DCM_OtherPatientIDsSequence), true);

    item = NULL;
    dataset.findOrCreateSequenceItem(DCM_OtherPatientIDsSequence, item, 0);

    // Testing LO
    {
    OFString value;
    item->findAndGetOFString(DCM_Manufacturer, value);
    BOOST_CHECK_EQUAL(value.c_str(), "MyManufacturer");
    }

    item = NULL;
    dataset.findOrCreateSequenceItem(DCM_OtherPatientIDsSequence, item, 1);

    // Testing LO
    {
    OFString value;
    item->findAndGetOFString(DCM_Manufacturer, value);
    BOOST_CHECK_EQUAL(value.c_str(), "MyManufacturer2");
    }
}

/*************************** TEST OK 06 *******************************/
/**
 * Nominal test case: Operator () Multiple Values
 */
struct TestDataOperatorBracketMultipleValues
{
   mongo::BSONObj bsonobject;

   TestDataOperatorBracketMultipleValues()
   {
       mongo::BSONObjBuilder bsonobjectbuilder;

       // Insert AE
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "AE";
       value_builder << "Value" << BSON_ARRAY("value1" << "value2");
       bsonobjectbuilder << "00080054" << value_builder.obj();
       }

       // Insert AS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "AS";
       value_builder << "Value" << BSON_ARRAY("value1" << "value2");
       bsonobjectbuilder << "00101010" << value_builder.obj();
       }

       // Insert CS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "CS";
       value_builder << "Value" << BSON_ARRAY("value1" << "value2");
       bsonobjectbuilder << "00080060" << value_builder.obj();
       }

       // Insert DA
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "DA";
       value_builder << "Value" << BSON_ARRAY("01/01/2001" << "02/02/2002");
       bsonobjectbuilder << "00100030" << value_builder.obj();
       }

       // Insert DS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "DS";
       value_builder << "Value" << BSON_ARRAY(60.5 << 61.6);
       bsonobjectbuilder << "00101030" << value_builder.obj();
       }

       // Insert DT
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "DT";
       value_builder << "Value" << BSON_ARRAY("01/01/2001 09:09:09" << "02/02/2002 09:09:09");
       bsonobjectbuilder << "00189074" << value_builder.obj();
       }

       // Insert FD
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "FD";
       value_builder << "Value" << BSON_ARRAY(42.5 << 43.6);
       bsonobjectbuilder << "00460044" << value_builder.obj();
       }

       // Insert FL
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "FL";
       value_builder << "Value" << BSON_ARRAY(15.2 << 16.3);
       bsonobjectbuilder << "00089459" << value_builder.obj();
       }

       // Insert IS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "IS";
       value_builder << "Value" << BSON_ARRAY(12 << 13);
       bsonobjectbuilder << "00082122" << value_builder.obj();
       }

       // Insert LO
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "LO";
       value_builder << "Value" << BSON_ARRAY("value1" << "value2");
       bsonobjectbuilder << "00080070" << value_builder.obj();
       }

       // Insert LT
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "LT";
       value_builder << "Value" << BSON_ARRAY("value1" << "value2");
       bsonobjectbuilder << "001021b0" << value_builder.obj();
       }

       // Insert PN
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "PN";
       value_builder << "Value" << BSON_ARRAY("Doe^John" << "Doe^Jane");
       bsonobjectbuilder << "00100010" << value_builder.obj();
       }

       // Insert SH
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SH";
       value_builder << "Value" << BSON_ARRAY("value1" << "value2");
       bsonobjectbuilder << "00102160" << value_builder.obj();
       }

       // Insert SL
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SL";
       value_builder << "Value" << BSON_ARRAY(10 << 11);
       bsonobjectbuilder << "00186020" << value_builder.obj();
       }

       // Insert SS
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "SS";
       value_builder << "Value" << BSON_ARRAY(11 << 12);
       bsonobjectbuilder << "00189219" << value_builder.obj();
       }

       // Insert ST
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "ST";
       value_builder << "Value" << BSON_ARRAY("value1" << "value2");
       bsonobjectbuilder << "00080081" << value_builder.obj();
       }

       // Insert TM
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "TM";
       value_builder << "Value" << BSON_ARRAY("08:08:08" << "09:09:09");
       bsonobjectbuilder << "00080013" << value_builder.obj();
       }

       // Insert UI
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "UI";
       value_builder << "Value" << BSON_ARRAY("1.2.3.4.5.6" << "1.2.3.4.5.7");
       bsonobjectbuilder << "00080016" << value_builder.obj();
       }

       // Insert UL
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "UL";
       value_builder << "Value" << BSON_ARRAY(6 << 7);
       bsonobjectbuilder << "00081161" << value_builder.obj();
       }

       // Insert US
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "US";
       value_builder << "Value" << BSON_ARRAY(5 << 6);
       bsonobjectbuilder << "00081197" << value_builder.obj();
       }

       // Insert UT
       {
       mongo::BSONObjBuilder value_builder;
       value_builder << "vr" << "UT";
       value_builder << "Value" << BSON_ARRAY("value1" << "value2");
       bsonobjectbuilder << "00287fe0" << value_builder.obj();
       }

       bsonobject = bsonobjectbuilder.obj();
   }

   ~TestDataOperatorBracketMultipleValues()
   {
       // Nothing to do
   }
};

BOOST_FIXTURE_TEST_CASE(OperatorBracketMultipleValues, TestDataOperatorBracketMultipleValues)
{
    dopamine::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset(bsonobject);

    OFCondition condition = EC_Normal;
    // Testing AE
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_RetrieveAETitle, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_RetrieveAETitle, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
    }

    // Testing AS
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_PatientAge, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_PatientAge, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
    }

    // Testing CS
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_Modality, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_Modality, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
    }

    // Testing DA
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_PatientBirthDate, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "01/01/2001");
    condition = dataset.findAndGetOFString(DCM_PatientBirthDate, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "02/02/2002");
    }

    // Testing DS
    {
    Float64 value;
    condition = dataset.findAndGetFloat64(DCM_PatientWeight, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_CLOSE(value, 60.5, 0.001);
    condition = dataset.findAndGetFloat64(DCM_PatientWeight, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_CLOSE(value, 61.6, 0.001);
    }

    // Testing DT
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_FrameAcquisitionDateTime, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "01/01/2001 09:09:09");
    condition = dataset.findAndGetOFString(DCM_FrameAcquisitionDateTime, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "02/02/2002 09:09:09");
    }

    // Testing FD
    {
    Float64 value;
    condition = dataset.findAndGetFloat64(DCM_PupilSize, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_CLOSE(value, 42.5, 0.001);
    condition = dataset.findAndGetFloat64(DCM_PupilSize, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_CLOSE(value, 43.6, 0.001);
    }

    // Testing FL
    {
    Float32 value;
    condition = dataset.findAndGetFloat32(DCM_RecommendedDisplayFrameRateInFloat, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_CLOSE(value, 15.2, 0.001);
    condition = dataset.findAndGetFloat32(DCM_RecommendedDisplayFrameRateInFloat, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_CLOSE(value, 16.3, 0.001);
    }

    // Testing IS
    {
    Sint32 value;
    condition = dataset.findAndGetSint32(DCM_StageNumber, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 12);
    condition = dataset.findAndGetSint32(DCM_StageNumber, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 13);
    }

    // Testing LO
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_Manufacturer, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_Manufacturer, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
    }

    // Testing LT
    // Be carefull: putAndInsertOFStringArray for LT add only 1 value !!!
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_AdditionalPatientHistory, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1\\value2");
    //condition = dataset.findAndGetOFString(DCM_AdditionalPatientHistory, value, 1);
    //BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    //BOOST_CHECK_EQUAL(value.c_str(), "value2");
    }

    // Testing PN
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_PatientName, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "Doe^John");
    condition = dataset.findAndGetOFString(DCM_PatientName, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "Doe^Jane");
    }

    // Testing SH
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_EthnicGroup, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1");
    condition = dataset.findAndGetOFString(DCM_EthnicGroup, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value2");
    }

    // Testing SL
    {
    Sint32 value;
    condition = dataset.findAndGetSint32(DCM_ReferencePixelX0, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 10);
    condition = dataset.findAndGetSint32(DCM_ReferencePixelX0, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 11);
    }

    // Testing SS
    {
    Sint16 value;
    condition = dataset.findAndGetSint16(DCM_TagAngleSecondAxis, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 11);
    condition = dataset.findAndGetSint16(DCM_TagAngleSecondAxis, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 12);
    }

    // Testing ST
    // Be carefull: putAndInsertOFStringArray for ST add only 1 value !!!
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_InstitutionAddress, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1\\value2");
    //condition = dataset.findAndGetOFString(DCM_InstitutionAddress, value, 1);
    //BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    //BOOST_CHECK_EQUAL(value.c_str(), "value2");
    }

    // Testing TM
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_InstanceCreationTime, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "08:08:08");
    condition = dataset.findAndGetOFString(DCM_InstanceCreationTime, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "09:09:09");
    }

    // Testing UI
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_SOPClassUID, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "1.2.3.4.5.6");
    condition = dataset.findAndGetOFString(DCM_SOPClassUID, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "1.2.3.4.5.7");
    }

    // Testing UL
    {
    Uint32 value;
    condition = dataset.findAndGetUint32(DCM_SimpleFrameList, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 6);
    condition = dataset.findAndGetUint32(DCM_SimpleFrameList, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 7);
    }

    // Testing US
    {
    Uint16 value;
    condition = dataset.findAndGetUint16(DCM_FailureReason, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 5);
    condition = dataset.findAndGetUint16(DCM_FailureReason, value, 1);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value, 6);
    }

    // Testing UT
    // Be carefull: putAndInsertOFStringArray for UT add only 1 value !!!
    {
    OFString value;
    condition = dataset.findAndGetOFString(DCM_PixelDataProviderURL, value, 0);
    BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    BOOST_CHECK_EQUAL(value.c_str(), "value1\\value2");
    //condition = dataset.findAndGetOFString(DCM_PixelDataProviderURL, value, 1);
    //BOOST_CHECK_EQUAL(condition == EC_Normal, true);
    //BOOST_CHECK_EQUAL(value.c_str(), "value2");
    }
}

/*************************** TEST OK XX *******************************/
/**
 * Nominal test case: EVR depending on context (EVR_ox, EVR_xs, EVR_lt)
 * WARNING: NOT IMPLEMENTED
 */
 
/*************************** TEST OK XX *******************************/
/**
 * Nominal test case: EVR OB, OF and OW
 * WARNING: NOT IMPLEMENTED
 */
 
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
    BOOST_REQUIRE_THROW(bsontodataset->set_specific_character_set("badvalue"),
                        std::runtime_error);
}

/*************************** TEST KO 02 *******************************/
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

BOOST_FIXTURE_TEST_CASE(TEST_KO_02, TestDataKO02)
{
    dopamine::BSONToDataSet bsontodataset;
    BOOST_REQUIRE_THROW(bsontodataset(bsonobject),
                        std::runtime_error);
}
