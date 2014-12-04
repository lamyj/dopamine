/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
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
    research_pacs::BSONToDataSet * bsontodataset;
 
    TestDataOK01()
    {
        bsontodataset = new research_pacs::BSONToDataSet();
    }
 
    ~TestDataOK01()
    {
        delete bsontodataset;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_01, TestDataOK01)
{
    // Object build
    BOOST_CHECK_EQUAL(bsontodataset != NULL, true);
    
    // Default value
    BOOST_CHECK_EQUAL(bsontodataset->get_specific_character_set() == "", true);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: set_specific_character_set
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK01)
{
    // set_specific_character_set
    bsontodataset->set_specific_character_set("ISO_IR 192");
    
    // Default value
    BOOST_CHECK_EQUAL(bsontodataset->get_specific_character_set() == "ISO_IR 192", true);
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: Operator ()
 */
 struct TestDataOK03
{
    research_pacs::BSONToDataSet * bsontodataset;
    mongo::BSONObjBuilder * bsonobjectbuilder;
 
    TestDataOK03()
    {
        bsontodataset = new research_pacs::BSONToDataSet();
        bsonobjectbuilder = new mongo::BSONObjBuilder();
        
        static char buffer[9];
        
        // Insert AE
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("AE");
        value_builder.append("test_AE");
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x0054);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert AS
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("AS");
        value_builder.append("test_AS");
        snprintf(buffer, 9, "%04x%04x", 0x0010, 0x1010);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert CS
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("CS");
        value_builder.append("value1");
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x0060);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert DA
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("DA");
        value_builder.append("01/01/2001");
        snprintf(buffer, 9, "%04x%04x", 0x0010, 0x0030);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert DS
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("DS");
        value_builder.append((Float64)60.5);
        snprintf(buffer, 9, "%04x%04x", 0x0010, 0x1030);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert DT
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("DT");
        value_builder.append("01/01/2001 09:09:09");
        snprintf(buffer, 9, "%04x%04x", 0x0018, 0x9074);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert FD
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("FD");
        value_builder.append((Float64)42.5);
        snprintf(buffer, 9, "%04x%04x", 0x0046, 0x0044);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert FL
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("FL");
        value_builder.append((Float32)15.2);
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x9459);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert IS
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("IS");
        value_builder.append((Sint32)12);
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x2122);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert LO
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("LO");
        value_builder.append("MyManufacturer");
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x0070);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert LT
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("LT");
        value_builder.append("test_valueLT");
        snprintf(buffer, 9, "%04x%04x", 0x0010, 0x21b0);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert PN
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("PN");
        value_builder.append("Doe^John");
        snprintf(buffer, 9, "%04x%04x", 0x0010, 0x0010);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert SH
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("SH");
        value_builder.append("test_valueSH");
        snprintf(buffer, 9, "%04x%04x", 0x0010, 0x2160);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert SL
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("SL");
        value_builder.append((Sint32)10);
        snprintf(buffer, 9, "%04x%04x", 0x0018, 0x6020);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert SS
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("SS");
        value_builder.append((Sint16)11);
        snprintf(buffer, 9, "%04x%04x", 0x0018, 0x9219);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert ST
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("ST");
        value_builder.append("MyAddress");
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x0081);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert TM
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("TM");
        value_builder.append("08:08:08");
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x0013);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert UI
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("UI");
        value_builder.append("1.2.3.4.5.6");
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x0016);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert UL
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("UL");
        value_builder.append((Uint32)6);
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x1161);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert US
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("US");
        value_builder.append((Uint16)5);
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x1197);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
        
        // Insert UT
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("UT");
        value_builder.append("test_valueUT");
        snprintf(buffer, 9, "%04x%04x", 0x0028, 0x7fe0);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
    }
 
    ~TestDataOK03()
    {
        delete bsontodataset;
        delete bsonobjectbuilder;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_03, TestDataOK03)
{
    DcmDataset dataset = (*bsontodataset)(bsonobjectbuilder->obj());
    
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
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: Insert empty value
 */
 struct TestDataOK04
{
    research_pacs::BSONToDataSet * bsontodataset;
    mongo::BSONObjBuilder * bsonobjectbuilder;
 
    TestDataOK04()
    {
        bsontodataset = new research_pacs::BSONToDataSet();
        bsonobjectbuilder = new mongo::BSONObjBuilder();
        
        static char buffer[9];
        
        // Insert CS
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("CS");
        value_builder.appendNull();
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x0060);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
    }
 
    ~TestDataOK04()
    {
        delete bsontodataset;
        delete bsonobjectbuilder;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_04, TestDataOK04)
{
    DcmDataset dataset = (*bsontodataset)(bsonobjectbuilder->obj());
    
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
    research_pacs::BSONToDataSet * bsontodataset;
    mongo::BSONObjBuilder * bsonobjectbuilder;
 
    TestDataOK05()
    {
        bsontodataset = new research_pacs::BSONToDataSet();
        bsonobjectbuilder = new mongo::BSONObjBuilder();
        
        static char buffer[9];
        
        mongo::BSONObjBuilder* objectcs = new mongo::BSONObjBuilder();
        // Insert CS
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("CS");
        value_builder.append("value1");
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x0060);
        (*objectcs) << buffer << value_builder.arr();
        }
        // Insert LO
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("LO");
        value_builder.append("MyManufacturer");
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x0070);
        (*objectcs) << buffer << value_builder.arr();
        }
        
        // Insert SQ
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("SQ");
        value_builder.append(objectcs->obj());
        snprintf(buffer, 9, "%04x%04x", 0x0010, 0x1002);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
    }
 
    ~TestDataOK05()
    {
        delete bsontodataset;
        delete bsonobjectbuilder;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_05, TestDataOK05)
{
    DcmDataset dataset = (*bsontodataset)(bsonobjectbuilder->obj());
    
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
}

/*************************** TEST OK 06 *******************************/
/**
 * Nominal test case: Multi-valued (VM > 1)
 */
 struct TestDataOK06
{
    research_pacs::BSONToDataSet * bsontodataset;
    mongo::BSONObjBuilder * bsonobjectbuilder;
 
    TestDataOK06()
    {
        bsontodataset = new research_pacs::BSONToDataSet();
        bsonobjectbuilder = new mongo::BSONObjBuilder();
        
        static char buffer[9];
        
        // Insert CS
        {
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("CS");
        mongo::BSONArrayBuilder value_builder2;
        value_builder2.append("value1");
        value_builder2.append("value2");
        value_builder.append(value_builder2.arr());
        snprintf(buffer, 9, "%04x%04x", 0x0008, 0x0060);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
        }
    }
 
    ~TestDataOK06()
    {
        delete bsontodataset;
        delete bsonobjectbuilder;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_OK_06, TestDataOK06)
{
    DcmDataset dataset = (*bsontodataset)(bsonobjectbuilder->obj());
    
    // Testing CS
    {
    OFString value;
    dataset.findAndGetOFStringArray(DCM_Modality, value);
    BOOST_CHECK_EQUAL(value.c_str(), "value1\\value2");
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
    research_pacs::BSONToDataSet * bsontodataset;
    mongo::BSONObjBuilder * bsonobjectbuilder;
 
    TestDataKO02()
    {
        bsontodataset = new research_pacs::BSONToDataSet();
        bsonobjectbuilder = new mongo::BSONObjBuilder();
        
        static char buffer[9];
        
        mongo::BSONArrayBuilder value_builder;
        value_builder.append("an");
        value_builder.append("temp");
        snprintf(buffer, 9, "%04x%04x", 0x9998, 0x9998);
        (*bsonobjectbuilder) << buffer << value_builder.arr();
    }
 
    ~TestDataKO02()
    {
        delete bsontodataset;
        delete bsonobjectbuilder;
    }
};

BOOST_FIXTURE_TEST_CASE(TEST_KO_02, TestDataKO02)
{
    BOOST_REQUIRE_THROW((*bsontodataset)(bsonobjectbuilder->obj()),
                        std::runtime_error);
}
