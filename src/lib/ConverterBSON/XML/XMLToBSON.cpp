/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>
#include <utility>

#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "core/ConverterBase64.h"
#include "core/ExceptionPACS.h"
#include "core/LoggerPACS.h"
#include "XMLToBSON.h"

namespace dopamine
{

namespace converterBSON
{

XMLToBSON
::XMLToBSON():
    ConverterBSONXML() // base class initialization
{
    // Nothing to do
}

XMLToBSON
::~XMLToBSON()
{
    // Nothing to do
}

mongo::BSONObj XMLToBSON::from_ptree(boost::property_tree::ptree tree)
{
    mongo::BSONObjBuilder builder;

    bool find_tag_nativedicommodel = false;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_nativedicommodel,
                  tree)
    {
        if (it_nativedicommodel.first != Tag_NativeDicomModel)
        {
            std::stringstream streamerror;
            streamerror << "Unkown main XML node " << it_nativedicommodel.first;
            throw ExceptionPACS(streamerror.str());
        }

        if (find_tag_nativedicommodel)
        {
            std::stringstream streamerror;
            streamerror << "Too many XML node " << Tag_NativeDicomModel;
            throw ExceptionPACS(streamerror.str());
        }

        find_tag_nativedicommodel = true;

        BOOST_FOREACH(boost::property_tree::ptree::value_type &it_dicomattribute,
                      it_nativedicommodel.second)
        {
            if (it_dicomattribute.first == "<xmlattr>") {} // ignore it
            else if (it_dicomattribute.first != Tag_DicomAttribute)
            {
                std::stringstream streamerror;
                streamerror << "Unkown XML sub-node " << it_dicomattribute.first
                            << " for XML node " << Tag_NativeDicomModel;
                throw ExceptionPACS(streamerror.str());
            }
            else
            {
                this->_add_element(it_dicomattribute.second, builder);
            }
        }
    }

    return builder.obj();
}

mongo::BSONObj
XMLToBSON
::from_string(const std::string &xml)
{
    boost::property_tree::ptree ptree;

    std::stringstream xmlstream;
    xmlstream << xml;
    boost::property_tree::read_xml(xmlstream, ptree);

    return this->from_ptree(ptree);
}

void
XMLToBSON
::_add_person_name(boost::property_tree::ptree tree,
                   mongo::BSONArrayBuilder &array_builder,
                   unsigned int count) const
{
    std::vector<std::pair<mongo::BSONObj, bool> > values; values.resize(count);
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_value,
                  tree)
    {
        if (it_value.first == Tag_PersonName)
        {
            unsigned int number = it_value.second.get<int>(Attribute_Number);

            bool isempty = true;
            mongo::BSONObjBuilder object_builder;
            BOOST_FOREACH(boost::property_tree::ptree::value_type &it_component_name,
                          it_value.second)
            {
                if (it_component_name.first == "<xmlattr>") {} // ignore it
                else if (it_component_name.first == Tag_Alphabetic ||
                         it_component_name.first == Tag_Ideographic ||
                         it_component_name.first == Tag_Phonetic)
                {
                    isempty = false;
                    this->_add_component_name(it_component_name.second,
                                              object_builder,
                                              it_component_name.first);
                }
                else
                {
                    std::stringstream stream;
                    stream << "Unkown XML sub-node '" << it_component_name.first
                           << "' for XML node " << Tag_PersonName;
                    throw ExceptionPACS(stream.str());
                }
            }

            values[number-1] = std::make_pair(object_builder.obj(),
                                              isempty);
        }
    }

    for (auto it = values.begin(); it != values.end(); ++it)
    {
        if ((*it).second)
        {
            array_builder.appendNull();
        }
        else
        {
            array_builder.append((*it).first);
        }
    }
}

void
XMLToBSON
::_add_sequence(boost::property_tree::ptree tree,
                mongo::BSONArrayBuilder &array_builder,
                unsigned int count) const
{

    std::vector<std::pair<mongo::BSONObj, bool> > values; values.resize(count);
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_item,
                  tree)
    {
        if (it_item.first == "<xmlattr>") {} // ignore it
        else if (it_item.first == Tag_Item)
        {
            unsigned int number = it_item.second.get<int>(Attribute_Number);

            boost::property_tree::ptree nativetree;
            nativetree.add_child(Tag_NativeDicomModel, it_item.second);

            XMLToBSON converter;
            mongo::BSONObj buildedobject = converter.from_ptree(nativetree);

            values[number-1] = std::make_pair(buildedobject,
                                              false);
        }
        else
        {
            std::stringstream stream;
            stream << "Unkown XML sub-node '" << it_item.first
                   << "' for XML node " << Tag_DicomAttribute;
            throw ExceptionPACS(stream.str());
        }
    }

    for (auto it = values.begin(); it != values.end(); ++it)
    {
        if ((*it).second)
        {
            array_builder.appendNull();
        }
        else
        {
            array_builder.append((*it).first);
        }
    }
}

void
XMLToBSON
::_add_component_name(boost::property_tree::ptree tree,
                      mongo::BSONObjBuilder &object_builder,
                      const std::string &tag_name) const
{
    std::string family_name = "";
    std::string given_name = "";
    std::string middle_name = "";
    std::string prefix_name = "";
    std::string suffix_name = "";
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_alpha,
                  tree)
    {
        if (it_alpha.first == Tag_FamilyName)
        {
            if (family_name != "")
            {
                std::stringstream stream;
                stream << "Too many XML sub-node '" << Tag_FamilyName
                       << "' for node '" << tag_name << "'";
                throw ExceptionPACS(stream.str());
            }

            family_name = it_alpha.second.data();
        }
        else if (it_alpha.first == Tag_GivenName)
        {
            if (given_name != "")
            {
                std::stringstream stream;
                stream << "Too many XML sub-node '" << Tag_GivenName
                       << "' for node '" << tag_name << "'";
                throw ExceptionPACS(stream.str());
            }

            given_name = it_alpha.second.data();
        }
        else if (it_alpha.first == Tag_MiddleName)
        {
            if (middle_name != "")
            {
                std::stringstream stream;
                stream << "Too many XML sub-node '" << Tag_MiddleName
                       << "' for node '" << tag_name << "'";
                throw ExceptionPACS(stream.str());
            }

            middle_name = it_alpha.second.data();
        }
        else if (it_alpha.first == Tag_NamePrefix)
        {
            if (prefix_name != "")
            {
                std::stringstream stream;
                stream << "Too many XML sub-node '" << Tag_NamePrefix
                       << "' for node '" << tag_name << "'";
                throw ExceptionPACS(stream.str());
            }

            prefix_name = it_alpha.second.data();
        }
        else if (it_alpha.first == Tag_NameSuffix)
        {
            if (suffix_name != "")
            {
                std::stringstream stream;
                stream << "Too many XML sub-node '" << Tag_NameSuffix
                       << "' for node '" << tag_name << "'";
                throw ExceptionPACS(stream.str());
            }

            suffix_name = it_alpha.second.data();
        }
        else
        {
            std::stringstream stream;
            stream << "Unkown XML sub-node '" << it_alpha.first
                   << "' for node '" << tag_name << "'";
            throw ExceptionPACS(stream.str());
        }
    }

    std::string result = "";
    if (suffix_name != "")
    {
        result = "^" + suffix_name;
    }
    if (result != "" || prefix_name != "")
    {
        result = "^" + prefix_name + result;
    }
    if (result != "" || middle_name != "")
    {
        result = "^" + middle_name + result;
    }
    if (result != "" || given_name != "")
    {
        result = "^" + given_name + result;
    }
    if (result != "" || family_name != "")
    {
        result = family_name + result;
    }

    object_builder << tag_name << result;
}

void
XMLToBSON
::_add_binary_data(boost::property_tree::ptree tree,
                   mongo::BSONObjBuilder &object_builder) const
{
    std::string const treedata = tree.data();
    std::string dec = ConverterBase64::decode(treedata);

    object_builder.appendBinData("data", dec.size(),
                          mongo::BinDataGeneral, dec.c_str());
}

template<>
void
XMLToBSON
::_add_value<Sint32>(boost::property_tree::ptree tree,
                     mongo::BSONArrayBuilder &array_builder,
                     unsigned int count) const
{
    std::vector<std::pair<Sint32, bool> > values; values.resize(count);
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_value,
                  tree)
    {
        if (it_value.first == Tag_Value)
        {
            unsigned int number = it_value.second.get<int>(Attribute_Number);
            values[number-1] = std::make_pair(it_value.second.get_value<Sint32>(),
                                              it_value.second.empty());
        }
    }

    for (auto it = values.begin(); it != values.end(); ++it)
    {
        if ((*it).second)
        {
            array_builder.appendNull();
        }
        else
        {
            mongo::BSONObjBuilder builder;
            builder.appendIntOrLL("number", (*it).first);
            mongo::BSONObj object = builder.obj();
            array_builder.append(object.getField("number").numberLong());
        }
    }
}

template<>
void
XMLToBSON
::_add_value<Uint32>(boost::property_tree::ptree tree,
                     mongo::BSONArrayBuilder &array_builder,
                     unsigned int count) const
{
    std::vector<std::pair<Uint32, bool> > values; values.resize(count);
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_value,
                  tree)
    {
        if (it_value.first == Tag_Value)
        {
            unsigned int number = it_value.second.get<int>(Attribute_Number);
            values[number-1] = std::make_pair(it_value.second.get_value<Uint32>(),
                                              it_value.second.empty());
        }
    }

    for (auto it = values.begin(); it != values.end(); ++it)
    {
        if ((*it).second)
        {
            array_builder.appendNull();
        }
        else
        {
            mongo::BSONObjBuilder builder;
            builder.appendIntOrLL("number", (*it).first);
            mongo::BSONObj object = builder.obj();
            array_builder.append(object.getField("number").numberLong());
        }
    }
}

template<typename TType>
void
XMLToBSON
::_add_value(boost::property_tree::ptree tree,
             mongo::BSONArrayBuilder &array_builder,
             unsigned int count) const
{
    std::vector<std::pair<TType, bool> > values; values.resize(count);
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_value,
                  tree)
    {
        if (it_value.first == Tag_Value)
        {
            unsigned int number = it_value.second.get<int>(Attribute_Number);
            values[number-1] = std::make_pair(it_value.second.get_value<TType>(),
                                              it_value.second.empty());
        }
    }

    for (auto it = values.begin(); it != values.end(); ++it)
    {
        if ((*it).second)
        {
            array_builder.appendNull();
        }
        else
        {
            array_builder.append((*it).first);
        }
    }
}

void
XMLToBSON
::_add_element(boost::property_tree::ptree tree,
               mongo::BSONObjBuilder &builder) const
{
    // Get attributes
    std::string const tag = tree.get<std::string>(Attribute_Tag);
    std::string const vr = tree.get<std::string>(Attribute_VR);
    auto keyword_ = tree.get_optional<std::string>(Attribute_Keyword);
    auto private_creator_ = tree.get_optional<std::string>(Attribute_PrivateCreator);

    // TODO: private creator is not yet implement

    // Get the corresponding DICOM tag
    std::string tagstr = tag;
    tagstr.insert(tagstr.begin() + 4, ',');
    DcmTag response;
    OFCondition condition = DcmTag::findTagFromName(tagstr.c_str(), response);
    if (condition.bad())
    {
        std::stringstream stream;
        stream << "Unknown DICOM Tag: " << tag
               << "; Error: " << condition.text();
        throw ExceptionPACS(stream.str());
    }

    if (vr != response.getVR().getValidVRName())
    {
        loggerWarning() << "VR is ambigous for tag " << response.getXTag()
                        << ". Use DcmVR = " << response.getVR().getValidVRName();
    }
    if (keyword_ && keyword_.get() != response.getTagName())
    {
        loggerWarning() << "Keyword is ambigous for tag " << response.getXTag();
    }

    unsigned int count_value_node = 0;
    unsigned int count_personname_node = 0;
    unsigned int count_item_node = 0;
    unsigned int count_inlinebinary_node = 0;

    mongo::BSONObjBuilder object_builder;
    mongo::BSONObjBuilder binary_data_builder;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &it_value,
                  tree)
    {
        if (it_value.first == "<xmlattr>")
        {
            // ignore it, process just before the foreach
        }
        else if (it_value.first == Tag_Value)        ++count_value_node;
        else if (it_value.first == Tag_PersonName)   ++count_personname_node;
        else if (it_value.first == Tag_Item)         ++count_item_node;
        else if (it_value.first == Tag_InlineBinary)
        {
            ++count_inlinebinary_node;
            this->_add_binary_data(it_value.second, binary_data_builder);
        }
        else if (it_value.first == Tag_BulkData)
        {
            throw ExceptionPACS("Not implemented yet");
        }
        else
        {
            std::stringstream stream;
            stream << "Unkown XML sub-node '" << it_value.first
                   << "' for XML node " << Tag_DicomAttribute;
            throw ExceptionPACS(stream.str());
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
        stream << "Too many different XML sub-node type for XML Node "
               << Tag_DicomAttribute;
        throw ExceptionPACS(stream.str());
    }

    if (count_inlinebinary_node > 1)
    {
        std::stringstream stream;
        stream << "Too many sub-node " << Tag_InlineBinary
               << " for node " << Tag_DicomAttribute;
        throw ExceptionPACS(stream.str());
    }

    object_builder << "vr" << response.getVR().getValidVRName();

    DcmEVR const evr = response.getVR().getValidEVR();
    if (count_value_node != 0 ||
        count_personname_node != 0 ||
        count_item_node != 0)
    {
        mongo::BSONArrayBuilder array_builder;

        if      (evr == EVR_AE) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_AS) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_AT) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_CS) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_DA) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_DS) this->_add_value<Float64>(tree, array_builder, count_value_node);
        else if (evr == EVR_DT) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_FD) this->_add_value<Float64>(tree, array_builder, count_value_node);
        else if (evr == EVR_FL) this->_add_value<Float32>(tree, array_builder, count_value_node);
        else if (evr == EVR_IS) this->_add_value<Sint32>(tree, array_builder, count_value_node);
        else if (evr == EVR_LO) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_LT) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_PN) this->_add_person_name(tree, array_builder, count_personname_node);
        else if (evr == EVR_SH) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_SL) this->_add_value<Sint32>(tree, array_builder, count_value_node);
        else if (evr == EVR_SQ) this->_add_sequence(tree, array_builder, count_item_node);
        else if (evr == EVR_SS) this->_add_value<Sint16>(tree, array_builder, count_value_node);
        else if (evr == EVR_ST) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_TM) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_UI) this->_add_value<std::string>(tree, array_builder, count_value_node);
        else if (evr == EVR_UL) this->_add_value<Uint32>(tree, array_builder, count_value_node);
        else if (evr == EVR_US) this->_add_value<Uint16>(tree, array_builder, count_value_node);
        else if (evr == EVR_UT) this->_add_value<std::string>(tree, array_builder, count_value_node);

        else
        {
            std::stringstream stream;
            stream << "Bad VR " << response.getVR().getValidVRName()
                   << " for node " << Tag_Value << " or " << Tag_PersonName
                   << " or " << Tag_Item;
            throw ExceptionPACS(stream.str());
        }

        object_builder << "Value" << array_builder.arr();

    }
    else if (count_inlinebinary_node != 0)
    {
        mongo::BSONObj object_binary = binary_data_builder.obj();
        if (object_binary.hasField("data"))
        {
            object_builder << Tag_InlineBinary << object_binary.getField("data");
        }
        else
        {
            object_builder.appendNull("InlineBinary");
        }
    }
    else
    {
        if (evr == EVR_OB || evr == EVR_OF ||  evr == EVR_OW || evr == EVR_UN)
        {
            object_builder.appendNull("InlineBinary");
        }
        else
        {
            object_builder.appendNull("Value");
        }
    }

    builder << tag << object_builder.obj();
}

} // namespace converterBSON

} // namespace dopamine
