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

#define BOOST_TEST_MODULE ModuleConvertBSONDataset
#include <boost/test/unit_test.hpp>

#include <dcmtk/dcmdata/dcdatset.h>

#include <mongo/bson/bson.h>

#include "ConverterBSON/BSONToDataSet.h"
#include "ConverterBSON/DataSetToBSON.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Converter BSON <-> Dataset
 */
struct TestDataConversionBSONDataset
{
   mongo::BSONObj bsonobject;

   TestDataConversionBSONDataset()
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

   ~TestDataConversionBSONDataset()
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

BOOST_FIXTURE_TEST_CASE(ConversionBSONDataset, TestDataConversionBSONDataset)
{
    // Convert original BSON to Dataset
    dopamine::BSONToDataSet bsontodataset;
    DcmDataset dataset = bsontodataset(bsonobject);

    // Convert Dataset to new BSON
    dopamine::DataSetToBSON datasettobson;
    mongo::BSONObjBuilder bsonobjbuilder;
    datasettobson(&dataset, bsonobjbuilder);
    mongo::BSONObj const newbson = bsonobjbuilder.obj();

    // Compare original BSON with new
    isEqual(bsonobject, newbson);
    // Compare new BSON with original
    isEqual(newbson, bsonobject);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Converter Dataset <-> BSON
 */
struct TestDataConversionDatasetBSON
{
    DcmDataset* dataset;

    TestDataConversionDatasetBSON()
    {
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
        DcmItem* item = new DcmItem(DCM_Item);
        item->putAndInsertOFStringArray(DCM_PatientID, "123");
        dataset->insertSequenceItem(DCM_OtherPatientIDsSequence, item);
    }

    ~TestDataConversionDatasetBSON()
    {
        delete dataset;
    }
};

void isEqual(DcmDataset & firstdataset, DcmDataset & seconddataset)
{
    DcmObject * it = NULL;
    while(NULL != (it = seconddataset.nextInContainer(it)))
    {
        DcmElement * element = NULL;
        OFCondition condition = firstdataset.findAndGetElement(it->getTag().getXTag(), element);
        BOOST_CHECK_EQUAL(condition == EC_Normal, true);
        BOOST_CHECK_EQUAL(element != NULL, true);

        DcmElement * newelement = NULL;
        condition = seconddataset.findAndGetElement(it->getTag().getXTag(), newelement);
        BOOST_CHECK_EQUAL(condition == EC_Normal, true);
        BOOST_CHECK_EQUAL(newelement != NULL, true);

        switch (element->getVR())
        {
        case EVR_SQ:
        {
            DcmSequenceOfItems * sequence = NULL;
            condition = firstdataset.findAndGetSequence(it->getTag().getXTag(), sequence);
            BOOST_CHECK_EQUAL(condition == EC_Normal, true);
            BOOST_CHECK_EQUAL(sequence != NULL, true);

            DcmSequenceOfItems * newsequence = NULL;
            condition = seconddataset.findAndGetSequence(it->getTag().getXTag(), newsequence);
            BOOST_CHECK_EQUAL(condition == EC_Normal, true);
            BOOST_CHECK_EQUAL(newsequence != NULL, true);

            DcmObject * itoldseq = NULL;
            DcmObject * itnewseq = NULL;
            itoldseq = sequence->nextInContainer(itoldseq);
            itnewseq = newsequence->nextInContainer(itnewseq);
            while(NULL != itoldseq && NULL != itnewseq)
            {
                DcmObject * itoldseqitem = NULL;
                DcmObject * itnewseqitem = NULL;
                itoldseqitem = itoldseq->nextInContainer(itoldseqitem);
                itnewseqitem = itnewseq->nextInContainer(itnewseqitem);
                while(NULL != itoldseqitem && NULL != itnewseqitem)
                {
                    std::stringstream stream1;
                    itoldseqitem->print(stream1);

                    std::stringstream stream2;
                    itnewseqitem->print(stream2);

                    BOOST_CHECK_EQUAL(stream1.str(), stream2.str());

                    itoldseqitem = itoldseq->nextInContainer(itoldseqitem);
                    itnewseqitem = itnewseq->nextInContainer(itnewseqitem);
                }
                // Check if both are finish
                BOOST_CHECK_EQUAL(itoldseqitem == NULL, true);
                BOOST_CHECK_EQUAL(itnewseqitem == NULL, true);

                //std::cout << "value1 = " << stream1.str() << std::endl;
                //std::cout << "value2 = " << stream2.str() << std::endl;
                itoldseq = sequence->nextInContainer(itoldseq);
                itnewseq = newsequence->nextInContainer(itnewseq);
            }
            // Check if both are finish
            BOOST_CHECK_EQUAL(itoldseq == NULL, true);
            BOOST_CHECK_EQUAL(itnewseq == NULL, true);

            break;
        }
        default:
        {
            OFString value1;
            condition = element->getOFString(value1, 0);
            BOOST_CHECK_EQUAL(condition == EC_Normal, true);
            OFString value2;
            condition = newelement->getOFString(value2, 0);
            BOOST_CHECK_EQUAL(condition == EC_Normal, true);
            BOOST_CHECK_EQUAL(value1.c_str(), value2.c_str());
        }
        }
    }
}

BOOST_FIXTURE_TEST_CASE(ConversionDatasetBSON, TestDataConversionDatasetBSON)
{
    // Convert original Dataset to BSON
    dopamine::DataSetToBSON datasettobson;
    mongo::BSONObjBuilder bsonobjbuilder;
    datasettobson(dataset, bsonobjbuilder);
    mongo::BSONObj const bsonobj = bsonobjbuilder.obj();

    // Convert BSON to new Dataset
    dopamine::BSONToDataSet bsontodataset;
    DcmDataset newdataset = bsontodataset(bsonobj);

    // Compare new Dataset with original
    isEqual(*dataset, newdataset);
    isEqual(newdataset, *dataset);
}
