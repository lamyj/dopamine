/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleBSONToDataSet
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <string>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include <mongo/bson/bson.h>

#include "ConverterBSON/XML/BSONToXML.h"
#include "core/ExceptionPACS.h"
#include "services/ServicesTools.h"

/****************************** TOOLS ***********************************/

// Declaration
void check_dicom_attribute(boost::property_tree::ptree &tree, const mongo::BSONObj &object);

// Check the xml nodes InlineBinary
void check_inline_binary(boost::property_tree::ptree & tree,
                         mongo::BSONObj const & object)
{
    int length = 0;
    std::string const initial_data = std::string(object.getField("InlineBinary").binDataClean(length));

    typedef
        boost::archive::iterators::transform_width<
            boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6
            > binary_t;

    std::string const treedata = tree.data();
    std::string dec(binary_t(treedata.begin()), binary_t(treedata.end()));

    BOOST_CHECK_EQUAL(dec, initial_data);
}

// Check the xml nodes Alphabetic, Ideographic and Phonetic
void check_name_component(boost::property_tree::ptree & tree,
                          mongo::BSONObj const & object)
{
    std::string family_name = "";
    std::string given_name = "";
    std::string middle_name = "";
    std::string prefix_name = "";
    std::string suffix_name = "";
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_alpha,
                  tree)
    {
        if (it_alpha.first == dopamine::Tag_FamilyName)
        {
            if (family_name != "")
            {
                std::stringstream stream;
                stream << "Too many XML sub-node '" << dopamine::Tag_FamilyName
                       << "' for node '" << dopamine::Tag_Alphabetic
                       << "' or '" << dopamine::Tag_Ideographic << "' or '"
                       << dopamine::Tag_Phonetic;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
            }

            family_name = it_alpha.second.data();
        }
        else if (it_alpha.first == dopamine::Tag_GivenName)
        {
            if (given_name != "")
            {
                std::stringstream stream;
                stream << "Too many XML sub-node '" << dopamine::Tag_GivenName
                       << "' for node '" << dopamine::Tag_Alphabetic
                       << "' or '" << dopamine::Tag_Ideographic << "' or '"
                       << dopamine::Tag_Phonetic;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
            }

            given_name = it_alpha.second.data();
        }
        else if (it_alpha.first == dopamine::Tag_MiddleName)
        {
            if (middle_name != "")
            {
                std::stringstream stream;
                stream << "Too many XML sub-node '" << dopamine::Tag_MiddleName
                       << "' for node '" << dopamine::Tag_Alphabetic
                       << "' or '" << dopamine::Tag_Ideographic << "' or '"
                       << dopamine::Tag_Phonetic;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
            }

            middle_name = it_alpha.second.data();
        }
        else if (it_alpha.first == dopamine::Tag_NamePrefix)
        {
            if (prefix_name != "")
            {
                std::stringstream stream;
                stream << "Too many XML sub-node '" << dopamine::Tag_NamePrefix
                       << "' for node '" << dopamine::Tag_Alphabetic
                       << "' or '" << dopamine::Tag_Ideographic << "' or '"
                       << dopamine::Tag_Phonetic;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
            }

            prefix_name = it_alpha.second.data();
        }
        else if (it_alpha.first == dopamine::Tag_NameSuffix)
        {
            if (suffix_name != "")
            {
                std::stringstream stream;
                stream << "Too many XML sub-node '" << dopamine::Tag_NameSuffix
                       << "' for node '" << dopamine::Tag_Alphabetic
                       << "' or '" << dopamine::Tag_Ideographic << "' or '"
                       << dopamine::Tag_Phonetic;
                BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
            }

            suffix_name = it_alpha.second.data();
        }
        else
        {
            std::stringstream stream;
            stream << "Unkown XML sub-node '" << it_alpha.first
                   << "' for node '" << dopamine::Tag_Alphabetic
                   << "' or '" << dopamine::Tag_Ideographic << "' or '"
                   << dopamine::Tag_Phonetic;
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
        }
    }

    std::string name = object.getField(dopamine::Tag_Alphabetic).String();
    std::vector<std::string> name_components;
    boost::split(name_components, name,
                 boost::is_any_of("^"), boost::token_compress_off);

    BOOST_CHECK_EQUAL((name_components.size() > 0 && family_name != ""), true);
    BOOST_CHECK_EQUAL((name_components.size() > 1 && given_name != ""), true);
    BOOST_CHECK_EQUAL((name_components.size() > 2 && middle_name != ""), true);
    BOOST_CHECK_EQUAL((name_components.size() > 3 && prefix_name != ""), true);
    BOOST_CHECK_EQUAL((name_components.size() > 4 && suffix_name != ""), true);

    while (name_components.size() != 5)
        name_components.push_back("");
    BOOST_CHECK_EQUAL(name_components[0], family_name);
    BOOST_CHECK_EQUAL(name_components[1], given_name);
    BOOST_CHECK_EQUAL(name_components[2], middle_name);
    BOOST_CHECK_EQUAL(name_components[3], prefix_name);
    BOOST_CHECK_EQUAL(name_components[4], suffix_name);
}

// Check Attributes of the xml node Value
void check_value_xmlattr(boost::property_tree::ptree & tree,
                         mongo::BSONObj const & object,
                         unsigned int const & number)
{
    bool findnumber = false;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_xmlattr,
                  tree)
    {
        if (it_xmlattr.first == "number")
        {
            std::stringstream stream;
            stream << number;
            BOOST_CHECK_EQUAL(it_xmlattr.second.data(), stream.str());
            findnumber = true;
        }
        else
        {
            std::stringstream stream;
            stream << "Unkown attribute '" << it_xmlattr.first
                   << "' for node " << dopamine::Tag_Value << " or "
                   << dopamine::Tag_PersonName << " or "
                   << dopamine::Tag_Item;
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
        }
    }

    if ( !findnumber )
    {
        std::stringstream stream;
        stream << "Missing mandatory attribute 'number' for node "
               << dopamine::Tag_Value;
        BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
    }
}

// Check the xml node Item
void check_item(boost::property_tree::ptree & tree,
                mongo::BSONObj const & object,
                unsigned int const & number)
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_xmlattr,
                  tree)
    {
        if (it_xmlattr.first == "<xmlattr>")
        {
            check_value_xmlattr(it_xmlattr.second, object, number);
        }
        else if (it_xmlattr.first == dopamine::Tag_DicomAttribute)
        {
            check_dicom_attribute(it_xmlattr.second, object.getField("Value").Array()[number-1].Obj());
        }
        else
        {
            std::stringstream stream;
            stream << "Unkown XML sub-node '" << it_xmlattr.first
                   << "' for node " << dopamine::Tag_Item;
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
        }
    }
}

// Check the xml node PersonName
void check_person_name(boost::property_tree::ptree & tree,
                       mongo::BSONObj const & object,
                       unsigned int const & number)
{
    unsigned int count_alphabetic = 0;
    unsigned int count_ideographic = 0;
    unsigned int count_phonetic = 0;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_xmlattr,
                  tree)
    {
        if (it_xmlattr.first == "<xmlattr>")
        {
            check_value_xmlattr(it_xmlattr.second, object, number);
        }
        else if (it_xmlattr.first == dopamine::Tag_Alphabetic)
        {
            ++count_alphabetic;
            check_name_component(it_xmlattr.second,
                                 object.getField("Value").Array()[number-1].Obj());
        }
        else if (it_xmlattr.first == dopamine::Tag_Ideographic)
        {
            ++count_ideographic;
            check_name_component(it_xmlattr.second,
                                 object.getField("Value").Array()[number-1].Obj());
        }
        else if (it_xmlattr.first == dopamine::Tag_Phonetic)
        {
            ++count_phonetic;
            check_name_component(it_xmlattr.second,
                                 object.getField("Value").Array()[number-1].Obj());
        }
        else
        {
            std::stringstream stream;
            stream << "Unkown XML sub-node '" << it_xmlattr.first
                   << "' for node " << dopamine::Tag_PersonName;
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
        }
    }

    if (count_alphabetic > 1 || count_ideographic > 1 || count_phonetic > 1)
    {
        std::stringstream stream;
        stream << "Too many XML sub-node '" << dopamine::Tag_Alphabetic
               << "' or '" << dopamine::Tag_Ideographic << "' or '"
               << dopamine::Tag_Phonetic <<"' for node " << dopamine::Tag_PersonName;
        BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
    }
}

// Check the xml node Value
void check_value(boost::property_tree::ptree & tree,
                 mongo::BSONObj const & object,
                 unsigned int const & number)
{
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_xmlattr,
                  tree)
    {
        if (it_xmlattr.first == "<xmlattr>")
        {
            check_value_xmlattr(it_xmlattr.second, object, number);
        }
        else
        {
            std::stringstream stream;
            stream << "Unkown XML sub-node '" << it_xmlattr.first
                   << "' for node " << dopamine::Tag_Value;
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
        }
    }

    std::string const value = tree.data();
    std::string const initial_value =
            dopamine::services::bsonelement_to_string(object.getField("Value").Array()[number-1]);

    BOOST_CHECK_EQUAL(value, initial_value);
}

// Check Attributes of the xml node DicomAttribute
void check_dicom_attribute_xmlattr(boost::property_tree::ptree & tree,
                                   mongo::BSONObj const & object,
                                   std::string const & tag)
{
    // Get the corresponding DICOM tag
    std::string tagstr = tag;
    tagstr.insert(tagstr.begin() + 4, ',');
    DcmTag response;
    OFCondition condition = DcmTag::findTagFromName(tagstr.c_str(), response);
    if (condition.bad())
    {
        std::stringstream stream;
        stream << "Unknown DICOM Tag: " << tag;
        BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
    }

    bool findvr, findtag = false;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_xmlattr,
                  tree)
    {
        if (it_xmlattr.first == "vr")
        {
            BOOST_CHECK_EQUAL(it_xmlattr.second.data(), response.getVR().getValidVRName());

            std::string const objectvr = object.getField("vr").String();
            BOOST_CHECK_EQUAL(it_xmlattr.second.data(), objectvr);

            findvr = true;
        }
        else if (it_xmlattr.first == "tag")
        {
            BOOST_CHECK_EQUAL(it_xmlattr.second.data(), tag);
            findtag = true;
        }
        else if (it_xmlattr.first == "keyword")
        {
            BOOST_CHECK_EQUAL(it_xmlattr.second.data(), response.getTagName());
        }
        else if (it_xmlattr.first == "privateCreator")
        {
            BOOST_CHECK_EQUAL(it_xmlattr.second.data(), response.getPrivateCreator());
        }
        else
        {
            std::stringstream stream;
            stream << "Unkown attribute '" << it_xmlattr.first
                   << "' for node " << dopamine::Tag_DicomAttribute;
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
        }
    }

    if ( !findvr )
    {
        std::stringstream stream;
        stream << "Missing mandatory attribute 'vr' for node "
               << dopamine::Tag_DicomAttribute;
        BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
    }

    if ( !findtag )
    {
        std::stringstream stream;
        stream << "Missing mandatory attribute 'tag' for node "
               << dopamine::Tag_DicomAttribute;
        BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
    }
}

// Check the xml node DicomAttribute
void check_dicom_attribute(boost::property_tree::ptree & tree,
                           mongo::BSONObj const & object)
{
    std::cout << "DEBUG RLA         check_dicom_attribute begin" << std::endl;
    std::string const tag = tree.get<std::string>(dopamine::Attribute_Tag);
    std::cout << "DEBUG RLA         check_dicom_attribute tag = " << tag << std::endl;

    unsigned int count_value_node = 0;
    unsigned int count_personname_node = 0;
    unsigned int count_item_node = 0;
    unsigned int count_inlinebinary_node = 0;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_value,
                  tree)
    {
        if (it_value.first == "<xmlattr>")
        {
            check_dicom_attribute_xmlattr(it_value.second, object.getField(tag).Obj(), tag);
        }
        else if (it_value.first == dopamine::Tag_Value)
        {
            ++count_value_node;
            check_value(it_value.second, object.getField(tag).Obj(), count_value_node);
        }
        else if (it_value.first == dopamine::Tag_PersonName)
        {
            ++count_personname_node;
            check_person_name(it_value.second, object.getField(tag).Obj(), count_personname_node);
        }
        else if (it_value.first == dopamine::Tag_Item)
        {
            ++count_item_node;
            check_item(it_value.second, object.getField(tag).Obj(), count_item_node);
        }
        else if (it_value.first == dopamine::Tag_InlineBinary)
        {
            ++count_inlinebinary_node;
            check_inline_binary(it_value.second, object.getField(tag).Obj());
        }
        else if (it_value.first == dopamine::Tag_BulkData)
        {
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS("Not implemented yet"));
        }
        else
        {
            std::stringstream stream;
            stream << "Unkown XML sub-node '" << it_value.first
                   << "' for node " << dopamine::Tag_DicomAttribute;
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
        }
    }

    int different_node = 0;
    if (count_value_node != 0)          ++different_node;
    if (count_personname_node != 0)     ++different_node;
    if (count_item_node != 0)           ++different_node;
    if (count_inlinebinary_node != 0)   ++different_node;

    if (different_node > 1)
    {
        std::stringstream stream;
        stream << "Node " << dopamine::Tag_DicomAttribute
               << " cannot contain different sub-node type";
        BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
    }

    if (count_inlinebinary_node > 1)
    {
        std::stringstream stream;
        stream << "Too many sub-node " << dopamine::Tag_InlineBinary
               << " for node " << dopamine::Tag_DicomAttribute;
        BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS(stream.str()));
    }
    std::cout << "DEBUG RLA         check_dicom_attribute end" << std::endl;
}

// Check the xml node NativeDicomModel
void check_native_dicom_model(boost::property_tree::ptree & tree,
                              mongo::BSONObj const & object)
{
    std::cout << "DEBUG RLA     check_native_dicom_model begin" << std::endl;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_dicomattribute,
                  tree)
    {
        BOOST_CHECK_EQUAL(it_dicomattribute.first, dopamine::Tag_DicomAttribute);

        check_dicom_attribute(it_dicomattribute.second, object);
    }
    std::cout << "DEBUG RLA     check_native_dicom_model end" << std::endl;
}

// Check the root xml node
void check_property_tree(boost::property_tree::ptree & tree,
                         mongo::BSONObj const & object)
{
    std::cout << "DEBUG RLA check_property_tree begin" << std::endl;
    bool find_tag_nativedicommodel = false;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_nativedicommodel,
                  tree)
    {
        if (find_tag_nativedicommodel)
        {
            BOOST_THROW_EXCEPTION(dopamine::ExceptionPACS("Find more than one NativeDicomModel tag"));
            return;
        }
        find_tag_nativedicommodel = true;
        BOOST_CHECK_EQUAL(it_nativedicommodel.first, dopamine::Tag_NativeDicomModel);

        check_native_dicom_model(it_nativedicommodel.second, object);
    }
    std::cout << "DEBUG RLA check_property_tree end" << std::endl;
}

/**************************** END TOOLS *********************************/

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::BSONToXML * bsontoxml = new dopamine::BSONToXML();

    BOOST_CHECK_EQUAL(bsontoxml != NULL, true);

    delete bsontoxml;
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueAS1" << "valueAS2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueAT1" << "valueAT2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueCS1" << "valueCS2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueDT1" << "valueDT2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueLO1" << "valueLO2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueLT1" << "valueLT2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
            BSON_ARRAY(BSON(dopamine::Tag_Alphabetic << "Doe^John^Wallas^Rev.^Chief Executive Officer")
                    << BSON(dopamine::Tag_Alphabetic << "Smith^Jane^Scarlett^Ms.^Goddess"));
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueSH1" << "valueSH2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Conversion VR SQ
 */
BOOST_AUTO_TEST_CASE(ConversionSQ)
{
    // Create 2 BSON with LO and CS tags
    std::string const tagLO = "00100020";
    std::string const vrLO = "LO";
    mongo::BSONArray const valuesLO = BSON_ARRAY("valueLO1" << "valueLO2");
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueST1" << "valueST2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueTM1" << "valueTM2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueUI1" << "valueUI2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
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
    mongo::BSONArray const values = BSON_ARRAY("valueUT1" << "valueUT2");
    mongo::BSONObj object = BSON(tag << BSON("vr" << vr << "Value" << values));

    // Conversion
    dopamine::BSONToXML bsontoxml;
    auto tree = bsontoxml(object);

    // Check result
    check_property_tree(tree, object);
}
