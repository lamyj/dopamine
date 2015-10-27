/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "bson_converter.h"
#include "core/ExceptionPACS.h"

namespace dopamine
{

struct ToBSONVisitor
{
    typedef mongo::BSONObj result_type;

    result_type operator()(dcmtkpp::VR const vr) const
    {
        result_type result;

        mongo::BSONObjBuilder builder;
        builder << "vr" << dcmtkpp::as_string(vr);
        result = builder.obj();

        return result;
    }

    template<typename T>
    result_type operator()(dcmtkpp::VR const vr, T const & value) const
    {
        result_type result;

        mongo::BSONObjBuilder builder;
        builder << "vr" << dcmtkpp::as_string(vr);

        mongo::BSONArrayBuilder array;
        for(auto const & item: value)
        {
            array << item;
        }
        builder << "Value" << array.arr();
        result = builder.obj();

        return result;
    }

    result_type operator()(dcmtkpp::VR const vr,
                           dcmtkpp::Value::Integers const & value) const
    {
        result_type result;

        mongo::BSONObjBuilder builder;
        builder << "vr" << dcmtkpp::as_string(vr);

        mongo::BSONArrayBuilder array;
        for(auto const & item: value)
        {
            array << static_cast<long long>(item);
        }
        builder << "Value" << array.arr();
        result = builder.obj();

        return result;
    }

    result_type operator()(dcmtkpp::VR const vr,
                           dcmtkpp::Value::Strings const & value) const
    {
        result_type result;

        mongo::BSONObjBuilder builder;
        builder << "vr" << dcmtkpp::as_string(vr);

        if(vr == dcmtkpp::VR::PN)
        {
            auto const fields = { "Alphabetic", "Ideographic", "Phonetic" };

            mongo::BSONArrayBuilder array;
            for(auto const item: value)
            {
                auto fields_it = fields.begin();

                mongo::BSONObjBuilder bson_item;

                std::string::size_type begin=0;
                while(begin != std::string::npos)
                {
                    std::string::size_type const end = item.find("=", begin);

                    std::string::size_type size = 0;
                    if(end != std::string::npos)
                    {
                        size = end-begin;
                    }
                    else
                    {
                        size = std::string::npos;
                    }

                    bson_item << *fields_it << item.substr(begin, size);

                    if(end != std::string::npos)
                    {
                        begin = end+1;
                        ++fields_it;
                        if(fields_it == fields.end())
                        {
                            throw ExceptionPACS("Invalid Person Name");
                        }
                    }
                    else
                    {
                        begin = end;
                    }
                }
                array << bson_item.obj();
            }
            builder << "Value" << array.arr();
        }
        else
        {
            mongo::BSONArrayBuilder array;
            for(auto const & item: value)
            {
                array << item;
            }
            builder << "Value" << array.arr();
        }
        result = builder.obj();

        return result;
    }

    result_type operator()(dcmtkpp::VR const vr,
                           dcmtkpp::Value::DataSets const & value) const
    {
        result_type result;

        mongo::BSONObjBuilder builder;
        builder << "vr" << dcmtkpp::as_string(vr);

        mongo::BSONArrayBuilder array;
        for(auto const & item: value)
        {
            array << as_bson(item);
        }
        builder << "Value" << array.arr();
        result = builder.obj();

        return result;
    }

    result_type operator()(dcmtkpp::VR const vr,
                           dcmtkpp::Value::Binary const & value) const
    {
        result_type result;

        mongo::BSONObjBuilder builder;
        builder << "vr" << dcmtkpp::as_string(vr);

        mongo::BSONObjBuilder binary_data_builder;
        binary_data_builder.appendBinData("data", value.size(),
                                          mongo::BinDataGeneral, &value[0]);
        builder << "InlineBinary" << binary_data_builder.obj().getField("data");
        result = builder.obj();

        return result;
    }

};

mongo::BSONObj as_bson(dcmtkpp::DataSet const & data_set)
{
    mongo::BSONObjBuilder object_builder;
    for(auto const & it: data_set)
    {
        auto const & tag = it.first;
        auto const & element = it.second;

        std::string const key(tag);
        auto const value = dcmtkpp::apply_visitor(ToBSONVisitor(), element);
        object_builder << key << value;
    }

    return object_builder.obj();
}

dcmtkpp::DataSet as_dataset(mongo::BSONObj const & bson)
{
    dcmtkpp::DataSet data_set;

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
        bool skip_field = (field_name.size() != 8);
        if(!skip_field)
        {
            for(std::string::const_iterator field_name_it=field_name.begin();
                field_name_it != field_name.end(); ++field_name_it)
            {
                if(!((*field_name_it>='0' && *field_name_it<='9') ||
                     (*field_name_it>='a' && *field_name_it<='f') ||
                     (*field_name_it>='A' && *field_name_it<='F')))
                {
                    skip_field = true;
                    break;
                }
            }
        }

        if(skip_field)
        {
            continue;
        }

        dcmtkpp::Tag const tag(field_name);

        // Value holding the VR and the data
        mongo::BSONObj const object = element_bson.Obj();

        // Get the VR : first item of value
        dcmtkpp::VR const vr = dcmtkpp::as_vr(object.getField("vr").String());

        dcmtkpp::Element element;

        if(vr == dcmtkpp::VR::AE || vr == dcmtkpp::VR::AS ||
           vr == dcmtkpp::VR::AT || vr == dcmtkpp::VR::CS ||
           vr == dcmtkpp::VR::DA || vr == dcmtkpp::VR::DT ||
           vr == dcmtkpp::VR::LO || vr == dcmtkpp::VR::LT ||
           vr == dcmtkpp::VR::SH || vr == dcmtkpp::VR::ST ||
           vr == dcmtkpp::VR::TM || vr == dcmtkpp::VR::UI ||
           vr == dcmtkpp::VR::UT)
        {
            element = dcmtkpp::Element(dcmtkpp::Value::Strings(), vr);

            auto const values = object.getField("Value").Array();
            for(auto const & bson_item: values)
            {
                element.as_string().push_back(bson_item.String());
            }
        }
        else if(vr == dcmtkpp::VR::PN)
        {
            element = dcmtkpp::Element(dcmtkpp::Value::Strings(), vr);

            auto const values = object.getField("Value").Array();
            for(auto const & bson_item: values)
            {
                dcmtkpp::Value::Strings::value_type dicom_item;
                auto const fields = { "Alphabetic", "Ideographic", "Phonetic" };
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

                element.as_string().push_back(dicom_item);
            }
        }
        else if(vr == dcmtkpp::VR::DS || vr == dcmtkpp::VR::FD ||
                vr == dcmtkpp::VR::FL)
        {
            element = dcmtkpp::Element(dcmtkpp::Value::Reals(), vr);

            auto const values = object.getField("Value").Array();
            for(auto const & bson_item: values)
            {
                element.as_real().push_back(bson_item.Double());
            }
        }
        else if(vr == dcmtkpp::VR::IS || vr == dcmtkpp::VR::SL ||
                vr == dcmtkpp::VR::SS || vr == dcmtkpp::VR::UL ||
                vr == dcmtkpp::VR::US)
        {
            element = dcmtkpp::Element(dcmtkpp::Value::Integers(), vr);

            auto const values = object.getField("Value").Array();
            for(auto const & bson_item: values)
            {
                if (bson_item.type() == mongo::BSONType::NumberLong)
                {
                    element.as_int().push_back(
                        static_cast<dcmtkpp::Value::Integers::value_type>(
                                    bson_item.Long()));
                }
                else if (bson_item.type() == mongo::BSONType::NumberDouble)
                {
                    element.as_int().push_back(
                        static_cast<dcmtkpp::Value::Integers::value_type>(
                                    bson_item.Double()));
                }
                else
                {
                    element.as_int().push_back(
                        static_cast<dcmtkpp::Value::Integers::value_type>(
                                    bson_item.Int()));
                }
            }
        }
        else if(vr == dcmtkpp::VR::SQ)
        {
            element = dcmtkpp::Element(dcmtkpp::Value::DataSets(), vr);

            auto const values = object.getField("Value").Array();
            for(auto const & bson_item: values)
            {
                auto const dicom_item = as_dataset(bson_item.Obj());
                element.as_data_set().push_back(dicom_item);
            }
        }
        else if(vr == dcmtkpp::VR::OB || vr == dcmtkpp::VR::OF ||
                vr == dcmtkpp::VR::OW || vr == dcmtkpp::VR::UN)
        {
            element = dcmtkpp::Element(dcmtkpp::Value::Binary(), vr);

            int size = 0;
            auto const values = object.getField("InlineBinary").binDataClean(size);

            element.as_binary().resize(size);
            std::copy(values, values+size, element.as_binary().begin());
        }
        else
        {
            throw ExceptionPACS("Unknown VR: "+dcmtkpp::as_string(vr));
        }

        data_set.add(tag, element);
    }

    return data_set;
}

} // namespace dopamine
