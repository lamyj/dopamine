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
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include <dcmtk/dcmdata/dctag.h>

#include "BSONToXML.h"
#include "core/ExceptionPACS.h"

namespace dopamine
{

BSONToXML
::BSONToXML()
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
::operator()(const mongo::BSONObj &bson)
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

bool
BSONToXML
::is_Dicom_field(const std::string &field_name)
{
    bool isdicom = (field_name.size() == 8);

    if(isdicom)
    {
        for(std::string::const_iterator field_name_it=field_name.begin();
            field_name_it!=field_name.end(); ++field_name_it)
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

void
BSONToXML
::_to_dicom_attribute(const mongo::BSONElement &bson,
                      boost::property_tree::ptree &tag_xml)
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

    tag_xml.put(Attribute_Tag, field_name);                                 // Mandatory
    tag_xml.put(Attribute_VR, bson.Obj().getField("vr").String());          // Mandatory
    tag_xml.put(Attribute_Keyword, response.getTagName());                  // Optional
    //tag_xml.put(Attribute_PrivateCreator, response.getPrivateCreator());  // Optional

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
    if     (evr == EVR_AE) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_AS) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_AT) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_CS) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_DA) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_DS) this->_to_value_string_number(element, tag_xml);                             // Number
    else if(evr == EVR_DT) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_FD) this->_to_value<double>(element, tag_xml, &mongo::BSONElement::Double);      // Number
    else if(evr == EVR_FL) this->_to_value<double>(element, tag_xml, &mongo::BSONElement::Double);      // Number
    else if(evr == EVR_IS) this->_to_value_string_number(element, tag_xml);                             // Number
    else if(evr == EVR_LO) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_LT) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_OB) this->_to_raw(element, tag_xml);                                             // Base64 encoded string
    // unknown VR !!! else if(evr == EVR_OD) this->_to_raw(element, tag_xml);                           // Base64 encoded string
    else if(evr == EVR_OF) this->_to_raw(element, tag_xml);                                             // Base64 encoded string
    else if(evr == EVR_OW) this->_to_raw(element, tag_xml);                                             // Base64 encoded string
    else if(evr == EVR_PN) this->_to_person_name(element, tag_xml);                                     // Object containing Person Name component groups as strings
    else if(evr == EVR_SH) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_SL) this->_to_value<int>(element, tag_xml, &mongo::BSONElement::Int);            // Number
    else if(evr == EVR_SQ) this->_to_item(element, tag_xml);                                            // Array containing DICOM JSON Objects
    else if(evr == EVR_SS) this->_to_value<int>(element, tag_xml, &mongo::BSONElement::Int);            // Number
    else if(evr == EVR_ST) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_TM) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_UI) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String
    else if(evr == EVR_UL) this->_to_value<int>(element, tag_xml, &mongo::BSONElement::Int);            // Number
    else if(evr == EVR_UN) this->_to_raw(element, tag_xml);                                             // Base64 encoded string
    else if(evr == EVR_US) this->_to_value<int>(element, tag_xml, &mongo::BSONElement::Int);            // Number
    else if(evr == EVR_UT) this->_to_value<std::string>(element, tag_xml, &mongo::BSONElement::String); // String

    // default
    else
    {
        throw std::runtime_error(std::string("Unhandled VR:") + vr.getValidVRName());
    }
}

void
BSONToXML
::_to_dicom_model(const mongo::BSONObj &bson,
                  boost::property_tree::ptree &tag_xml)
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
        if (!this->is_Dicom_field(field_name))
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
::_to_item(const mongo::BSONElement &bson,
           boost::property_tree::ptree &tag_xml)
{
    std::vector<mongo::BSONElement> elements = bson.Array();

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
::_to_person_name(const mongo::BSONElement &bson,
                  boost::property_tree::ptree &tag_xml)
{
    std::vector<mongo::BSONElement> elements = bson.Array();

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

                std::string name = it->Obj().getField(Tag_Alphabetic).String();
                std::vector<std::string> name_components;
                boost::split(name_components, name, boost::is_any_of("^"), boost::token_compress_off);

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
::_to_raw(const mongo::BSONElement &bson,
          boost::property_tree::ptree &tag_xml)
{
    // process only one item
    boost::property_tree::ptree tag_inlinebinary;

    int size=0;
    char const * begin = bson.binDataClean(size);

    typedef boost::archive::iterators::base64_from_binary<
                boost::archive::iterators::transform_width<
                    const char *, 6, 8>
                >
            base64_t;

    std::stringstream os;
    std::copy(
            base64_t(begin),
            base64_t(begin + size),
            boost::archive::iterators::ostream_iterator<char>(os)
        );

    tag_inlinebinary.put_value(os.str());

    tag_xml.add_child(Tag_InlineBinary, tag_inlinebinary);
}

void
BSONToXML
::_to_value_string_number(const mongo::BSONElement &bson,
                          boost::property_tree::ptree &tag_xml) const
{
    std::vector<mongo::BSONElement> elements = bson.Array();

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

template<typename TBSONType>
void
BSONToXML
::_to_value(mongo::BSONElement const & bson,
            boost::property_tree::ptree & tag_xml,
            typename BSONGetterType<TBSONType>::Type getter) const
{
    std::vector<mongo::BSONElement> elements = bson.Array();

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

} // namespace dopamine
