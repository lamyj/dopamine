/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/bson_converter.h"

#include <string>

#include <mongo/bson/bson.h>
#include <odil/DataSet.h>
#include <odil/Exception.h>
#include <odil/registry.h>
#include <odil/unicode.h>
#include <odil/Value.h>
#include <odil/VR.h>

#include "dopamine/Exception.h"

namespace
{

struct AsBSON
{
public:
    typedef mongo::BSONObj result_type;

    AsBSON(odil::Value::Strings const & specific_char_set)
    : _specific_character_set(specific_char_set)
    {
        // Nothing else
    }

    result_type operator()(odil::VR const vr) const
    {
        return BSON("vr" << odil::as_string(vr));
    }

    template<typename T>
    result_type operator()(odil::VR const vr, T const & value) const
    {
        mongo::BSONArrayBuilder array;
        for(auto const & item: value)
        {
            array << item;
        }

        return BSON("vr" << odil::as_string(vr) << "Value" << array.arr());
    }

    result_type operator()(
        odil::VR const vr, odil::Value::Integers const & value) const
    {
        mongo::BSONArrayBuilder array;
        for(auto const & item: value)
        {
            array << static_cast<long long>(item);
        }

        return BSON("vr" << odil::as_string(vr) << "Value" << array.arr());
    }

    result_type operator()(
        odil::VR const vr, odil::Value::Strings const & value) const
    {
        mongo::BSONArrayBuilder array;

        if(vr == odil::VR::PN)
        {
            for(auto const & item: value)
            {
                array << this->_convert_pn(item);
            }
        }
        else
        {
            for(auto const & item: value)
            {
                array << this->_convert_string(vr, item);
            }
        }

        return BSON("vr" << odil::as_string(vr) << "Value" << array.arr());
    }

    result_type operator()(
        odil::VR const vr, odil::Value::DataSets const & value) const
    {
        mongo::BSONArrayBuilder array;
        for(auto const & item: value)
        {
            array << dopamine::as_bson(item, this->_specific_character_set);
        }

        return BSON("vr" << odil::as_string(vr) << "Value" << array.arr());
    }

    result_type operator()(
        odil::VR const vr, odil::Value::Binary const & value) const
    {
        mongo::BSONArrayBuilder array;
        for(auto const & item: value)
        {
            mongo::BSONObjBuilder item_builder;
            item_builder.appendBinData(
                "data", item.size(), mongo::BinDataGeneral, &item[0]);
            array.append(item_builder.obj()["data"]);
        }

        return BSON("vr" << odil::as_string(vr) << "InlineBinary" << array.arr());
    }

private:
    /// Character Set
    odil::Value::Strings _specific_character_set;

    std::string _convert_string(
        odil::VR const vr, odil::Value::String const & value) const
    {
        if(
            vr != odil::VR::LO && vr != odil::VR::LT &&
            vr != odil::VR::PN && vr != odil::VR::SH &&
            vr != odil::VR::ST && vr != odil::VR::UT)
        {
            // Nothing to do
            return value;
        }

        return odil::as_utf8(
            value, this->_specific_character_set, vr==odil::VR::PN);
    }

    mongo::BSONObj _convert_pn(odil::Value::String const & value) const
    {
        static auto const fields = { "Alphabetic", "Ideographic", "Phonetic" };

        auto fields_it = fields.begin();

        mongo::BSONObjBuilder bson;

        std::string::size_type begin=0;
        while(begin != std::string::npos)
        {
            std::string::size_type const end = value.find("=", begin);

            std::string::size_type size = 0;
            if(end != std::string::npos)
            {
                size = end-begin;
            }
            else
            {
                size = std::string::npos;
            }

            auto const component = value.substr(begin, size);
            bson << *fields_it << this->_convert_string(odil::VR::PN, component);

            if(end != std::string::npos)
            {
                begin = end+1;
                ++fields_it;
                if(fields_it == fields.end())
                {
                    throw dopamine::Exception("Invalid Person Name");
                }
            }
            else
            {
                begin = end;
            }
        }

        return bson.obj();
    }
};

}

namespace dopamine
{

mongo::BSONObj as_bson(
    odil::DataSet const & data_set,
    odil::Value::Strings const & specific_character_set)
{
    auto current_specific_char_set = specific_character_set;

    mongo::BSONObjBuilder builder;
    for(auto const & item: data_set)
    {
        auto const & tag = item.first;
        auto const & element = item.second;

        if(tag == odil::registry::SpecificCharacterSet)
        {
            current_specific_char_set = element.as_string();
        }

        if(tag.element == 0)
        {
            // Group length, do nothing
            continue;
        }

        AsBSON const visitor(current_specific_char_set);
        builder << std::string(tag) << odil::apply_visitor(visitor, element);
    }

    return builder.obj();
}

odil::DataSet as_dataset(
    mongo::BSONObj const & bson,
    odil::Value::Strings const & specific_character_set)
{
    auto current_specific_char_set = specific_character_set;

    odil::DataSet data_set;

    for(auto it = bson.begin(); it.more();)
    {
        auto const bson_element = it.next();

        if(bson_element.isNull())
        {
            // Skip null elements (might happen when bson is a query result)
            continue;
        }

        // Skip elements that do not look like DICOM tags
        odil::Tag tag;
        try
        {
            tag = bson_element.fieldName();
        }
        catch(odil::Exception const &)
        {
            continue;
        }

        mongo::BSONObj const object = bson_element.Obj();
        odil::Element dicom_element;
        auto const vr = odil::as_vr(object.getField("vr").String());

        if(object.hasField("Value") && !object.getField("Value").isNull())
        {
            auto const values = object.getField("Value").Array();

            if(odil::is_string(vr))
            {
                dicom_element = odil::Element(odil::Value::Strings(), vr);
                if(vr != odil::VR::PN)
                {
                    for(auto const & item: values)
                    {
                        dicom_element.as_string().push_back(
                            odil::as_specific_character_set(
                                item.String(), current_specific_char_set));
                    }
                }
                else
                {
                    for(auto const & bson_item: values)
                    {
                        odil::Value::Strings::value_type dicom_item;
                        auto const fields = {
                            "Alphabetic", "Ideographic", "Phonetic" };
                        for(auto const & field: fields)
                        {
                            if(bson_item.Obj().hasField(field))
                            {
                                dicom_item += bson_item.Obj().getField(field).String();
                            }
                            dicom_item += "=";
                        }

                        while(*dicom_item.rbegin() == '=')
                        {
                            dicom_item = dicom_item.substr(0, dicom_item.size()-1);
                        }

                        dicom_element.as_string().push_back(
                            odil::as_specific_character_set(
                                dicom_item, current_specific_char_set));
                    }
                }
            }
            else if(odil::is_real(vr))
            {
                dicom_element = odil::Element(odil::Value::Reals(), vr);

                for(auto const & bson_item: values)
                {
                    dicom_element.as_real().push_back(bson_item.Double());
                }
            }
            else if(odil::is_int(vr))
            {
                dicom_element = odil::Element(odil::Value::Integers(), vr);

                for(auto const & bson_item: values)
                {
                    odil::Value::Integers::value_type dicom_item;
                    if(bson_item.type() == mongo::BSONType::NumberLong)
                    {
                        dicom_item = bson_item.Long();
                    }
                    else if (bson_item.type() == mongo::BSONType::NumberDouble)
                    {
                        dicom_item = bson_item.Double();
                    }
                    else
                    {
                        dicom_item = bson_item.Int();
                    }
                    dicom_element.as_int().push_back(dicom_item);
                }
            }
            else if(vr == odil::VR::SQ)
            {
                dicom_element = odil::Element(odil::Value::DataSets(), vr);

                for(auto const & bson_item: values)
                {
                    auto const dicom_item = as_dataset(
                        bson_item.Obj(), current_specific_char_set);
                    dicom_element.as_data_set().push_back(dicom_item);
                }
            }
        }
        else if(object.hasField("InlineBinary") && !object.getField("InlineBinary").isNull())
        {
            dicom_element = odil::Element(odil::Value::Binary(), vr);
            auto const values = object.getField("InlineBinary").Array();
            for(auto const & bson_item: values)
            {
                int size=0;
                char const * const begin = bson_item.binDataClean(size);
                dicom_element.as_binary().emplace_back(begin, begin+size);
            }
        }
        // Otherwise the element is empty, do nothing

        data_set.add(tag, dicom_element);

        if(tag == odil::registry::SpecificCharacterSet)
        {
            current_specific_char_set = dicom_element.as_string();
        }
    }

    return data_set;
}

}
