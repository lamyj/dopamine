/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/archive/mongo_query.h"

#include <functional>
#include <string>

#include <mongo/bson/bson.h>
#include <odil/DataSet.h>
#include <odil/message/Response.h>
#include <odil/registry.h>
#include <odil/SCP.h>
#include <odil/Tag.h>
#include <odil/VR.h>

#include "dopamine/bson_converter.h"

namespace
{

/// @brief Replace every occurence of old by new_.
std::string
replace(std::string const & value, std::string const & old, std::string const & new_)
{
    std::string result(value);
    size_t begin=0;
    while(std::string::npos != (begin=result.find(old, begin)))
    {
        result = result.replace(begin, old.size(), new_);
        if(begin+new_.size()<result.size())
        {
            begin = begin+new_.size();
        }
        else
        {
            begin = std::string::npos;
        }
    }

    return result;
}

}

namespace dopamine
{

namespace archive
{

void as_mongo_query(
    odil::DataSet const & data_set,
    mongo::BSONArrayBuilder & query_terms,
    mongo::BSONObjBuilder & query_fields)
{
    if(
        !data_set.has(odil::registry::QueryRetrieveLevel) ||
        data_set.empty(odil::registry::QueryRetrieveLevel))
    {
        odil::DataSet status_fields;
        status_fields.add(
            odil::registry::AttributeIdentifierList,
            { odil::registry::QueryRetrieveLevel });
        throw odil::SCP::Exception(
            "Missing Query/Retrieve Level",
            odil::message::Response::MissingAttribute, status_fields);
    }

    auto const & query_retrieve_level = data_set.as_string(
        odil::registry::QueryRetrieveLevel, 0);

    // Query with DICOM syntax matches
    auto dicom_query = as_bson(data_set);
    // Remove unused elements
    dicom_query = dicom_query.removeField("00080005"); // SpecificCharacterSet
    dicom_query = dicom_query.removeField("00080052"); // QueryRetrieveLevel

    // Build the MongoDB query and query fields from the DICOM query
    for(auto it = dicom_query.begin(); it.more(); /* nothing */)
    {
        auto const element = it.next();
        auto const object = element.Obj();

        auto const vr = object.getField("vr").String();

        std::string const field =
            odil::is_binary(odil::as_vr(vr))?"InlineBinary":"Value";

        if(object.hasField(field))
        {
            auto value = object.getField(field);
            auto const array = value.Array();
            if(array.empty())
            {
                throw odil::SCP::Exception(
                    "Empty query array",
                    odil::message::Response::ProcessingFailure);
            }
            if(array.size() == 1)
            {
                value = array[0];
            }

            // Convert the DICOM query term to MongoDB syntax
            mongo::BSONObjBuilder term;
            auto const converter = get_query_converter(
                get_match_type(vr, value));
            converter(
                std::string(element.fieldName())+"."+field, vr, value, term);
            query_terms << term.obj();
        }

        // Include the query fields in the results.
        query_fields << element.fieldName() << 1;
    }

    // Always include mandatory fields
    std::vector<odil::Tag> mandatory_fields = {
        odil::registry::PatientID,
    };

    if(query_retrieve_level=="STUDY" || query_retrieve_level=="SERIES" ||
        query_retrieve_level=="IMAGE")
    {
        mandatory_fields.push_back(odil::registry::StudyInstanceUID);
    }
    if(query_retrieve_level=="SERIES" || query_retrieve_level=="IMAGE")
    {
        mandatory_fields.push_back(odil::registry::SeriesInstanceUID);
    }
    if(query_retrieve_level=="IMAGE")
    {
        mandatory_fields.push_back(odil::registry::SOPInstanceUID);
    }
    for(auto const & field: mandatory_fields)
    {
        std::string const field_as_string(field);
        if(!query_fields.hasField(field_as_string))
        {
            query_fields << field_as_string << 1;
        }
    }
}

// Define Unknown specialization first, since other specializations use it.
template<>
void
as_mongo_query<MatchType::Unknown>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value, mongo::BSONObjBuilder & builder)
{
    if(vr == "OB" || vr == "OD" ||vr == "OF" || vr == "OL" ||vr == "OW" ||
       vr == "UN")
    {
        int length = 0;
        char const * data = value.binDataClean(length);
        builder.appendBinData(field, length, mongo::BinDataGeneral, data);
    }
    else
    {
        // Default action: convert to string
        builder << field << value.String();
    }
}

template<>
void
as_mongo_query<MatchType::SingleValue>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value, mongo::BSONObjBuilder & builder)
{
    if(vr == "PN" && value.type() == mongo::BSONType::Object)
    {
        if(value.Obj().hasField("Alphabetic"))
        {
            builder
                << field+".Alphabetic"
                << value.Obj().getField("Alphabetic").String();
            return;
        }
    }

    auto const odil_vr = odil::as_vr(vr);
    if(odil::is_real(odil_vr))
    {
        builder << field << value.Double();
    }
    else if(odil::is_int(odil_vr))
    {
        builder.appendIntOrLL(field, value.Int());
    }
    else
    {
        builder << field << value.String();
    }
}

template<>
void
as_mongo_query<MatchType::ListOfUID>(
    std::string const & field, std::string const & /* unused */,
    mongo::BSONElement const & value, mongo::BSONObjBuilder & builder)
{
    builder << field << BSON("$in" << value);
}

template<>
void
as_mongo_query<MatchType::Universal>(
    std::string const & /* unused */, std::string const & /* unused */,
    mongo::BSONElement const & /* unused */,
    mongo::BSONObjBuilder & /* unused */)
{
    // Universal is not part of the MongoDB query : do nothing
}

template<>
void
as_mongo_query<MatchType::WildCard>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value, mongo::BSONObjBuilder & builder)
{
    std::string regex;
    if(vr == "PN" && value.type() == mongo::BSONType::Object)
    {
        if(value.Obj().hasField("Alphabetic"))
        {
            regex = value.Obj().getField("Alphabetic").String();
        }
        else
        {
            regex = value.String();
        }
    }
    else
    {
        regex = value.String();
    }

    // Convert DICOM regex to PCRE: replace "*" by ".*", "?" by ".",
    // and escape other special PCRE characters (these are :
    // \^$.[]()+{}
    //
    // Escape "\\" first since we're using it to replace "."
    regex = replace(regex, "\\", "\\\\");
    // Escape "." first since we're using it to replace "*"
    regex = replace(regex, ".", "\\.");
    regex = replace(regex, "*", ".*");
    regex = replace(regex, "?", ".");
    // Escape other PCRE metacharacters
    regex = replace(regex, "^", "\\^");
    regex = replace(regex, "$", "\\$");
    regex = replace(regex, "[", "\\[");
    regex = replace(regex, "]", "\\]");
    regex = replace(regex, "(", "\\(");
    regex = replace(regex, ")", "\\)");
    regex = replace(regex, "+", "\\+");
    regex = replace(regex, "{", "\\{");
    regex = replace(regex, "}", "\\}");
    // Add the start and end anchors
    regex = "^"+regex+"$";

    // Use case-insensitive match for PN
    builder.appendRegex(
        field+(vr=="PN"?".Alphabetic":""), regex, (vr=="PN")?"i":"");
}

template<>
void
as_mongo_query<MatchType::Range>(
    std::string const & field, std::string const & /* unused */,
    mongo::BSONElement const & value, mongo::BSONObjBuilder & builder)
{
    auto const range = value.String();
    auto const separator = range.find("-");
    auto const begin = range.substr(0, separator);
    auto const end = range.substr(separator+1, std::string::npos);

    mongo::BSONObjBuilder range_object;
    if(!begin.empty())
    {
        range_object << "$gte" << begin;
    }
    if(!end.empty())
    {
        range_object << "$lte" << end;
    }

    builder << field << range_object.obj();
}

template<>
void
as_mongo_query<MatchType::MultipleValues>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value, mongo::BSONObjBuilder & builder)
{
    mongo::BSONArrayBuilder or_builder;
    for(auto const & item: value.Array())
    {
        auto converter = get_query_converter(get_match_type(vr, item));
        mongo::BSONObjBuilder item_builder;
        converter(field, vr, item, item_builder);

        or_builder << item_builder.obj();
    }
    builder << "$or" << or_builder.arr();
}

MatchType
get_match_type(std::string const & vr, mongo::BSONElement const & element)
{
    MatchType type = MatchType::Unknown;

    if(element.isNull())
    {
        // C.2.2.2.3 Universal Matching
        // Value is zero-length
        type = MatchType::Universal;
    }
    else
    {
        bool const is_date_or_time = (vr == "DA" || vr == "DT" || vr == "TM");
        bool const has_wildcard_matching = (
            !is_date_or_time &&
            vr != "SL" && vr != "SS" && vr != "UL" && vr != "US" &&
            vr != "FD" && vr != "FL" &&
            vr != "OB" && vr != "OD" && vr != "OF" && vr != "OL" &&
            vr != "OW" && vr != "UN" &&
            vr != "AT" && vr != "DS" && vr != "IS" && vr != "AS" &&
            vr != "UI");
        if(
            element.type() == mongo::String ||
            (element.type() == mongo::Object && vr == "PN"))
        {
            std::string value;
            if(element.type() == mongo::Object && vr == "PN")
            {
                value = element.Obj().getField("Alphabetic").String();
            }
            else
            {
                value = element.String();
            }

            bool const wildcard_in_value =
                (value.find_first_of("?*") != std::string::npos);
            bool const range_in_value =
                (value.find('-') != std::string::npos);

            if(has_wildcard_matching && wildcard_in_value)
            {
                // C.2.2.2.4 Wild Card Matching
                // Not a date, time, datetime, SL, SL, UL, US, FL, FD, OB,
                // OW, UN, AT, DS, IS, AS, UI and Value contains "*" or "?"
                type = MatchType::WildCard;
            }
            else if(is_date_or_time && range_in_value)
            {
                // C.2.2.2.5 Range Matching
                // Date, time or datetime, contains "-"
                type = MatchType::Range;
            }
            else
            {
                // C.2.2.2.1 Single Value Matching
                // Non-zero length AND
                //   not a date or time or datetime, contains no wildcard OR
                //   a date or time or datetime, contains a single date or time
                //   or datetime with not "-"
                type = MatchType::SingleValue;
            }
        }
        else if(element.isNumber())
        {
            type = MatchType::SingleValue;
        }
        else if(element.type() == mongo::Array && vr == "UI")
        {
            // C.2.2.2.2 List of UID Matching
            type = MatchType::ListOfUID;
        }
        else if(element.type() == mongo::Array && vr == "SQ")
        {
            // C.2.2.2.6 Sequence Matching
            type = MatchType::Sequence;
        }
        else if(element.type() == mongo::Array && vr != "SQ")
        {
            type = MatchType::MultipleValues;
        }
    }

    return type;
}

QueryConverter get_query_converter(MatchType match_type)
{
    QueryConverter converter;
    if(match_type == MatchType::SingleValue)
    {
        converter = as_mongo_query<MatchType::SingleValue>;
    }
    else if(match_type == MatchType::ListOfUID)
    {
        converter = as_mongo_query<MatchType::ListOfUID>;
    }
    else if(match_type == MatchType::Universal)
    {
        converter = as_mongo_query<MatchType::Universal>;
    }
    else if(match_type == MatchType::WildCard)
    {
        converter = as_mongo_query<MatchType::WildCard>;
    }
    else if(match_type == MatchType::Range)
    {
        converter = as_mongo_query<MatchType::Range>;
    }
    //else if(match_type == MatchType::Sequence)
    //{
    //    converter = as_mongo_query<MatchType::Sequence>;
    //}
    else if(match_type == MatchType::MultipleValues)
    {
        converter = as_mongo_query<MatchType::MultipleValues>;
    }
    else
    {
        converter = as_mongo_query<MatchType::Unknown>;
    }

    return converter;
}

} // namespace archive

} // namespace dopamine
