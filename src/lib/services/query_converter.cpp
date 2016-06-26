/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "query_converter.h"

#include <cstdint>
#include <functional>
#include <string>

#include <boost/lexical_cast.hpp>
#include <mongo/client/dbclient.h>

#include "dbconnection/MongoDBConnection.h"

namespace
{

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

namespace services
{

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
            builder << field << value.Obj().getField("Alphabetic").String();
            return;
        }
    }

    if(vr == "DS" || vr == "FD")
    {
        builder << field
            << boost::lexical_cast<double>(MongoDBConnection::as_string(value));
    }
    else if(vr == "FL")
    {
        builder << field
            << boost::lexical_cast<float>(MongoDBConnection::as_string(value));
    }
    else if(vr == "IS" || vr == "SL")
    {
        builder.appendIntOrLL(
            field,
            boost::lexical_cast<uint32_t>(MongoDBConnection::as_string(value)));
    }
    else if(vr == "SS")
    {
        builder << field
            << boost::lexical_cast<int16_t>(MongoDBConnection::as_string(value));
    }
    else if(vr == "UL")
    {
        builder.appendIntOrLL(
            field,
            boost::lexical_cast<uint32_t>(MongoDBConnection::as_string(value)));
    }
    else if(vr == "US")
    {
        builder << field
            << boost::lexical_cast<uint16_t>(MongoDBConnection::as_string(value));
    }
    else
    {
        builder << field << value.String();
    }
}

template<>
void
as_mongo_query<MatchType::ListOfUID>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value, mongo::BSONObjBuilder & builder)
{
    mongo::BSONArrayBuilder or_builder;
    for(auto const & item: value.Array())
    {
        or_builder << BSON(field << (item));
    }
    builder << "$or" << or_builder.arr();
}

template<>
void
as_mongo_query<MatchType::Universal>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value, mongo::BSONObjBuilder & builder)
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
    builder.appendRegex(field, regex, (vr=="PN")?"i":"");
}

template<>
void
as_mongo_query<MatchType::Range>(
    std::string const & field, std::string const & vr,
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
        bool const has_wildcard_matching = (!is_date_or_time &&
            vr != "SL" && vr != "SS" && vr != "UL" &&
            vr != "FD" && vr != "FL" &&
            vr != "OB" && vr != "OD" && vr != "OF" && vr != "OL" && vr != "OW" &&
            vr != "UN" &&
            vr != "DS" && vr != "US" &&
            vr != "UI");
        if(element.type() == mongo::String ||
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
            // Not a date or time, no wildcard AND date or time, no range
            if(!(!is_date_or_time &&
                 value.find_first_of("?*") != std::string::npos) &&
               !(is_date_or_time && value.find('-') != std::string::npos))
            {
                // C.2.2.2.1 Single Value Matching
                // Non-zero length AND
                //   not a date or time or datetime, contains no wildcard OR
                //   a date or time or datetime, contains a single date or time
                //   or datetime with not "-"
                type = MatchType::SingleValue;
            }
            else if(has_wildcard_matching &&
                value.find_first_of("?*") != std::string::npos)
            {
                // C.2.2.2.4 Wild Card Matching
                // Not a date, time, datetime, SL, SL, UL, US, FL, FD, OB,
                // OW, UN, AT, DS, IS, AS, UI and Value contains "*" or "?"
                type = MatchType::WildCard;
            }
            else if(is_date_or_time && value.find('-') != std::string::npos)
            {
                // C.2.2.2.5 Range Matching
                // Date, time or datetime, contains "-"
                type = MatchType::Range;
            }
        }
        else if(element.type() == mongo::NumberDouble ||
            element.type() == mongo::NumberInt ||
            element.type() == mongo::NumberLong)
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

}

}
