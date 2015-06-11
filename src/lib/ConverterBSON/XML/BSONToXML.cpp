/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <dcmtk/dcmdata/dctag.h>

#include "BSONToXML.h"
#include "core/ConverterBase64.h"
#include "core/ExceptionPACS.h"

namespace dopamine
{

namespace converterBSON
{

BSONToXML
::BSONToXML():
    ConverterBSONXML() // base class initialization
{
    // Nothing to do
}

BSONToXML
::~BSONToXML()
{
    // Nothing to do
}

boost::property_tree::ptree
BSONToXML
::to_ptree(mongo::BSONObj const & bson) const
{
    // root element
    boost::property_tree::ptree bsonxml;

    // XML dataset element
    boost::property_tree::ptree nativedicommodel;

    this->_to_dicom_model(bson, nativedicommodel);

    // Add XML dataset into root element
    bsonxml.add_child(Tag_NativeDicomModel, nativedicommodel);

    return bsonxml;
}

std::string
BSONToXML
::to_string(mongo::BSONObj const & bson) const
{
    boost::property_tree::ptree const ptree = this->to_ptree(bson);

    std::stringstream xmldataset;
    boost::property_tree::xml_writer_settings<char> settings(' ', 4);
    boost::property_tree::write_xml(xmldataset, ptree, settings);

    return xmldataset.str();
}

bool
BSONToXML
::is_dicom_field(std::string const & field_name) const
{
    bool isdicom = (field_name.size() == 8);

    if(isdicom)
    {
        for(std::string::const_iterator field_name_it = field_name.begin();
            field_name_it != field_name.end(); ++field_name_it)
        {
            if(!((*field_name_it>='0' && *field_name_it<='9') ||
                 (*field_name_it>='a' && *field_name_it<='f') ||
                 (*field_name_it>='A' && *field_name_it<='F')))
            {
                isdicom = false;
                break;
            }
        }
    }

    return isdicom;
}


// Warning: problem with
// mongo::DBClientConnection::query => convert int into double
template<>
void
BSONToXML
::_to_value<int>(mongo::BSONElement const & bson, std::string const & vr,
            boost::property_tree::ptree & tag_xml,
            typename BSONGetterType<int>::Type getter) const
{
    std::vector<mongo::BSONElement> const elements = bson.Array();

    unsigned int count = 0;

    for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
        it != elements.end(); ++it)
    {
        ++count;
        boost::property_tree::ptree tag_value;
        tag_value.put(Attribute_Number, count); // Mandatory

        if (!it->isNull())
        {
            tag_value.put_value((*it).numberInt());
        }

        tag_xml.add_child(Tag_Value, tag_value);
    }
}

template<typename TBSONType>
void
BSONToXML
::_to_value(mongo::BSONElement const & bson, std::string const & vr,
            boost::property_tree::ptree & tag_xml,
            typename BSONGetterType<TBSONType>::Type getter) const
{
    std::vector<mongo::BSONElement> const elements = bson.Array();

    unsigned int count = 0;

    for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
        it != elements.end(); ++it)
    {
        ++count;
        boost::property_tree::ptree tag_value;
        tag_value.put(Attribute_Number, count); // Mandatory

        if (!it->isNull())
        {
            tag_value.put_value(static_cast<TBSONType>(((*it).*getter)()));
        }

        tag_xml.add_child(Tag_Value, tag_value);
    }
}

void
BSONToXML
::_to_dicom_attribute(mongo::BSONElement const & bson,
                      boost::property_tree::ptree &tag_xml) const
{
    // Create : <DicomAttribute tag="GGGGEEEE" vr="VR" keyword="...."/>

    std::string const field_name = bson.fieldName();

    // Get the corresponding DICOM tag
    std::string tagstr = field_name;
    tagstr.insert(tagstr.begin() + 4, ',');
    DcmTag response;
    OFCondition condition = DcmTag::findTagFromName(tagstr.c_str(), response);
    if (condition.bad())
    {
        std::stringstream stream;
        stream << "Unknown DICOM Tag: " << field_name;
        throw ExceptionPACS(stream.str());
    }

    tag_xml.put(Attribute_Tag,
                field_name);                         // Mandatory
    tag_xml.put(Attribute_VR,
                bson.Obj().getField("vr").String()); // Mandatory
    tag_xml.put(Attribute_Keyword,
                response.getTagName());              // Optional
    //tag_xml.put(Attribute_PrivateCreator,
    //            response.getPrivateCreator());     // Optional

    // Add values
    mongo::BSONElement element;
    if (bson.Obj().hasField("Value"))
    {
        element = bson.Obj().getField("Value");

        if(element.isNull() || element.Array().size() == 0)
        {
            return;
        }
    }
    else if (bson.Obj().hasField("InlineBinary"))
    {
        element = bson.Obj().getField("InlineBinary");

        if(element.isNull() || element.type() != mongo::BSONType::BinData)
        {
            return;
        }
    }
    else
    {
        return;
    }

    // Get the VR : first item of value
    DcmVR const vr(bson.Obj().getField("vr").String().c_str());
    DcmEVR const evr(vr.getEVR());

    // See PS3.18 Section F.2 Table F.2.3-1 DICOM VR to JSON Data Type Mapping
    if     (evr == EVR_AE)      // String
    {
        this->_to_value<std::string>(element, "AE", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_AS)      // String
    {
        this->_to_value<std::string>(element, "AS", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_AT)      // String
    {
        this->_to_value<std::string>(element, "AT", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_CS)      // String
    {
        this->_to_value<std::string>(element, "CS", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_DA)      // String
    {
        this->_to_value<std::string>(element, "DA", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_DS)      // Number
    {
        this->_to_value_string_number(element, tag_xml);
    }
    else if(evr == EVR_DT)      // String
    {
        this->_to_value<std::string>(element, "DT", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_FD)      // Number
    {
        this->_to_value<double>(element, "FD", tag_xml,
                                &mongo::BSONElement::Double);
    }
    else if(evr == EVR_FL)      // Number
    {
        this->_to_value<double>(element, "FL", tag_xml,
                                &mongo::BSONElement::Double);
    }
    else if(evr == EVR_IS)      // Number
    {
        this->_to_value_string_number(element, tag_xml);
    }
    else if(evr == EVR_LO)      // String
    {
        this->_to_value<std::string>(element, "LO", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_LT)      // String
    {
        this->_to_value<std::string>(element, "LT", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_OB)      // Base64 encoded string
    {
        this->_to_raw(element, tag_xml);
    }
    else if(evr == EVR_OF)      // Base64 encoded string
    {
        this->_to_raw(element, tag_xml);
    }
    else if(evr == EVR_OW)      // Base64 encoded string
    {
        this->_to_raw(element, tag_xml);
    }
    else if(evr == EVR_PN)      // Contains Person Name component as strings
    {
        this->_to_person_name(element, tag_xml);
    }
    else if(evr == EVR_SH)      // String
    {
        this->_to_value<std::string>(element, "SH", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_SL)      // Number
    {
        this->_to_value<long long>(element, "SL", tag_xml,
                                   &mongo::BSONElement::Long);
    }
    else if(evr == EVR_SQ)      // Array containing DICOM JSON Objects
    {
        this->_to_item(element, tag_xml);
    }
    else if(evr == EVR_SS)      // Number
    {
        this->_to_value<int>(element, "SS", tag_xml,
                             &mongo::BSONElement::Int);
    }
    else if(evr == EVR_ST)      // String
    {
        this->_to_value<std::string>(element, "ST", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_TM)      // String
    {
        this->_to_value<std::string>(element, "TM", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_UI)      // String
    {
        this->_to_value<std::string>(element, "UI", tag_xml,
                                     &mongo::BSONElement::String);
    }
    else if(evr == EVR_UL)      // Number
    {
        this->_to_value<long long>(element, "UL", tag_xml,
                                   &mongo::BSONElement::Long);
    }
    else if(evr == EVR_UN)      // Base64 encoded string
    {
        this->_to_raw(element, tag_xml);
    }
    else if(evr == EVR_US)      // Number
    {
        this->_to_value<int>(element, "US", tag_xml,
                             &mongo::BSONElement::Int);
    }
    else if(evr == EVR_UT)      // String
    {
        this->_to_value<std::string>(element, "UT", tag_xml,
                                     &mongo::BSONElement::String);
    }

    // default
    else
    {
        std::stringstream streamerror;
        streamerror << "Unhandled VR: " << vr.getValidVRName();
        throw std::runtime_error(streamerror.str());
    }
}

void
BSONToXML
::_to_dicom_model(mongo::BSONObj const & bson,
                  boost::property_tree::ptree & tag_xml) const
{
    // Foreach bson element
    for(mongo::BSONObj::iterator it = bson.begin(); it.more();)
    {
        mongo::BSONElement const element_bson = it.next();

        if(element_bson.isNull())
        {
            // Skip null elements (might happen when bson is a query result)
            continue;
        }

        // Skip elements that do not look like DICOM tags
        std::string const field_name = element_bson.fieldName();
        if (!this->is_dicom_field(field_name))
        {
            continue;
        }

        boost::property_tree::ptree dicomattribute;
        this->_to_dicom_attribute(element_bson, dicomattribute);
        tag_xml.add_child(Tag_DicomAttribute, dicomattribute);
    }
}

void
BSONToXML
::_to_item(mongo::BSONElement const & bson,
           boost::property_tree::ptree & tag_xml) const
{
    std::vector<mongo::BSONElement> const elements = bson.Array();

    unsigned int count = 0;

    for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
        it != elements.end(); ++it)
    {
        ++count;
        boost::property_tree::ptree tag_item;
        tag_item.put(Attribute_Number, count); // Mandatory

        if (!it->isNull())
        {
            this->_to_dicom_model(it->Obj(), tag_item);
        }

        tag_xml.add_child(Tag_Item, tag_item);
    }
}

void
BSONToXML
::_to_person_name(mongo::BSONElement const & bson,
                  boost::property_tree::ptree & tag_xml) const
{
    std::vector<mongo::BSONElement> const elements = bson.Array();

    unsigned int count = 0;
    for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
        it != elements.end(); ++it)
    {
        ++count;
        boost::property_tree::ptree tag_personname;
        tag_personname.put(Attribute_Number, count); // Mandatory

        if (!it->isNull())
        {
            if (it->Obj().hasField(Tag_Alphabetic))
            {
                boost::property_tree::ptree tag_alphabetic;

                std::string const name =
                        it->Obj().getField(Tag_Alphabetic).String();
                std::vector<std::string> name_components;
                boost::split(name_components, name, boost::is_any_of("^"),
                             boost::token_compress_off);

                if (name_components.size() > 0)
                {
                    boost::property_tree::ptree tag_familyname;
                    tag_familyname.put_value(name_components[0]);
                    tag_alphabetic.add_child(Tag_FamilyName, tag_familyname);
                }
                if (name_components.size() > 1)
                {
                    boost::property_tree::ptree tag_givenname;
                    tag_givenname.put_value(name_components[1]);
                    tag_alphabetic.add_child(Tag_GivenName, tag_givenname);
                }
                if (name_components.size() > 2)
                {
                    boost::property_tree::ptree tag_middlename;
                    tag_middlename.put_value(name_components[2]);
                    tag_alphabetic.add_child(Tag_MiddleName, tag_middlename);
                }
                if (name_components.size() > 3)
                {
                    boost::property_tree::ptree tag_nameprefix;
                    tag_nameprefix.put_value(name_components[3]);
                    tag_alphabetic.add_child(Tag_NamePrefix, tag_nameprefix);
                }
                if (name_components.size() > 4)
                {
                    boost::property_tree::ptree tag_namesuffix;
                    tag_namesuffix.put_value(name_components[4]);
                    tag_alphabetic.add_child(Tag_NameSuffix, tag_namesuffix);
                }

                tag_personname.add_child(Tag_Alphabetic, tag_alphabetic);
            }
        }

        tag_xml.add_child(Tag_PersonName, tag_personname);
    }
}

void
BSONToXML
::_to_raw(mongo::BSONElement const & bson,
          boost::property_tree::ptree & tag_xml) const
{
    // process only one item
    boost::property_tree::ptree tag_inlinebinary;

    int size=0;
    char const * begin = bson.binDataClean(size);

    std::string const encode = ConverterBase64::encode(std::string(begin, size));

    tag_inlinebinary.put_value(encode);

    tag_xml.add_child(Tag_InlineBinary, tag_inlinebinary);
}

void
BSONToXML
::_to_value_string_number(mongo::BSONElement const & bson,
                          boost::property_tree::ptree &tag_xml) const
{
    std::vector<mongo::BSONElement> const elements = bson.Array();

    unsigned int count = 0;

    for(std::vector<mongo::BSONElement>::const_iterator it=elements.begin();
        it != elements.end(); ++it)
    {
        ++count;
        boost::property_tree::ptree tag_value;
        tag_value.put(Attribute_Number, count); // Mandatory

        if (!it->isNull())
        {
            if(it->type() == mongo::NumberDouble)
            {
                tag_value.put_value(it->Double());
            }
            else if(it->type() == mongo::NumberInt)
            {
                tag_value.put_value(it->Int());
            }
            else if(it->type() == mongo::NumberLong)
            {
                tag_value.put_value(it->Long());
            }
            else
            {
                // Throw an exception if it is not a String
                tag_value.put_value(it->String());
            }
        }

        tag_xml.add_child(Tag_Value, tag_value);
    }
}

} // namespace converterBSON

} // namespace dopamine
