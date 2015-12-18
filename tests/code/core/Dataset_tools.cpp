/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleDatasetTools
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/registry.h>

#include "core/dataset_tools.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: dataset to json string
 */
BOOST_AUTO_TEST_CASE(DatasetToJSON)
{
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"sopinstanceuid"},
                dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::PatientID, {"patientid"}, dcmtkpp::VR::LO);

    auto const dataset_string = dopamine::dataset_to_json_string(dataset);

    std::stringstream expected_result;
    expected_result << "{\n"
                    << "   \"00080018\" : {\n"
                    << "      \"Value\" : [ \"sopinstanceuid\" ],\n"
                    << "      \"vr\" : \"UI\"\n"
                    << "   },\n"
                    << "   \"00100020\" : {\n"
                    << "      \"Value\" : [ \"patientid\" ],\n"
                    << "      \"vr\" : \"LO\"\n"
                    << "   }\n"
                    << "}\n";

    BOOST_REQUIRE_EQUAL(dataset_string, expected_result.str());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: dataset to xml string
 */
BOOST_AUTO_TEST_CASE(DatasetToXML)
{
    dcmtkpp::DataSet dataset;
    dataset.add(dcmtkpp::registry::SOPInstanceUID, {"sopinstanceuid"},
                dcmtkpp::VR::UI);
    dataset.add(dcmtkpp::registry::PatientID, {"patientid"}, dcmtkpp::VR::LO);

    auto const dataset_string = dopamine::dataset_to_xml_string(dataset);

    std::stringstream expected_result;
    expected_result << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                    << "<NativeDicomModel>\n"
                    << "    <DicomAttribute vr=\"UI\" tag=\"00080018\" keyword=\"SOPInstanceUID\">\n"
                    << "        <Value number=\"1\">sopinstanceuid</Value>\n"
                    << "    </DicomAttribute>\n"
                    << "    <DicomAttribute vr=\"LO\" tag=\"00100020\" keyword=\"PatientID\">\n"
                    << "        <Value number=\"1\">patientid</Value>\n"
                    << "    </DicomAttribute>\n"
                    << "</NativeDicomModel>\n";

    BOOST_REQUIRE_EQUAL(dataset_string, expected_result.str());
}
