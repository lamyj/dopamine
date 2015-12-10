/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/Response.h>

#include "core/LoggerPACS.h"
#include "GeneratorPACS.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace services
{

template<>
void
GeneratorPACS
::_add_value_to_builder<Sint32>(mongo::BSONObjBuilder &builder,
                                std::string const & field,
                                std::string const & value) const
{
    // Fix compilation error for i386
    builder.appendIntOrLL(field, boost::lexical_cast<Sint32>(value));
}

template<>
void
GeneratorPACS
::_add_value_to_builder<Uint32>(mongo::BSONObjBuilder &builder,
                                std::string const & field,
                                std::string const & value) const
{
    // Fix compilation error for i386
    builder.appendIntOrLL(field, boost::lexical_cast<Uint32>(value));
}

template<typename TType>
void
GeneratorPACS
::_add_value_to_builder(mongo::BSONObjBuilder &builder,
                        std::string const & field,
                        std::string const & value) const
{
    builder << field << boost::lexical_cast<TType>(value);
}

// Define Unknown specialization first, since other specializations use it.
template<>
void
GeneratorPACS
::_dicom_query_to_mongo_query<GeneratorPACS::Match::Unknown>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    if (vr == "OB" || vr == "OF" || vr == "OW" || vr == "UN")
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
GeneratorPACS
::_dicom_query_to_mongo_query<GeneratorPACS::Match::SingleValue>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    if (vr == "PN" && value.type() == mongo::BSONType::Object)
    {
        if (value.Obj().hasField("Alphabetic"))
        {
            builder << field << value.Obj().getField("Alphabetic").String();
            return;
        }
    }

    if      (vr == "DS") this->_add_value_to_builder<Float64>(builder, field,
                                                              bsonelement_to_string(value));
    else if (vr == "FD") this->_add_value_to_builder<Float64>(builder, field,
                                                              bsonelement_to_string(value));
    else if (vr == "FL") this->_add_value_to_builder<Float32>(builder, field,
                                                              bsonelement_to_string(value));
    else if (vr == "IS") this->_add_value_to_builder<Sint32>(builder, field,
                                                             bsonelement_to_string(value));
    else if (vr == "SL") this->_add_value_to_builder<Sint32>(builder, field,
                                                             bsonelement_to_string(value));
    else if (vr == "SS") this->_add_value_to_builder<Sint16>(builder, field,
                                                             bsonelement_to_string(value));
    else if (vr == "UL") this->_add_value_to_builder<Uint32>(builder, field,
                                                             bsonelement_to_string(value));
    else if (vr == "US") this->_add_value_to_builder<Uint16>(builder, field,
                                                             bsonelement_to_string(value));
    else
    {
        builder << field << value.String();
    }
}

template<>
void
GeneratorPACS
::_dicom_query_to_mongo_query<GeneratorPACS::Match::ListOfUID>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    mongo::BSONArrayBuilder or_builder;
    std::vector<mongo::BSONElement> const or_terms = value.Array();
    for(std::vector<mongo::BSONElement>::const_iterator or_it = or_terms.begin();
        or_it != or_terms.end(); ++or_it)
    {
        or_builder << BSON(field << (*or_it));
    }
    builder << "$or" << or_builder.arr();
}

template<>
void
GeneratorPACS
::_dicom_query_to_mongo_query<GeneratorPACS::Match::Universal>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    // Universal is not part of the MongoDB query : do nothing
}

template<>
void
GeneratorPACS
::_dicom_query_to_mongo_query<GeneratorPACS::Match::WildCard>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    std::string regex;
    if (vr == "PN" && value.type() == mongo::BSONType::Object)
    {
        if (value.Obj().hasField("Alphabetic"))
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
GeneratorPACS
::_dicom_query_to_mongo_query<GeneratorPACS::Match::Range>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    std::string const range = value.String();
    std::size_t const separator = range.find("-");
    std::string const begin = range.substr(0, separator);
    std::string const end = range.substr(separator+1, std::string::npos);

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
GeneratorPACS
::_dicom_query_to_mongo_query<GeneratorPACS::Match::MultipleValues>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    mongo::BSONArrayBuilder or_builder;
    std::vector<mongo::BSONElement> or_terms = value.Array();
    for(std::vector<mongo::BSONElement>::const_iterator or_it = or_terms.begin();
        or_it != or_terms.end(); ++or_it)
    {
        Match::Type const match_type = this->_get_match_type(vr, *or_it);

        DicomQueryToMongoQuery function =
                this->_get_query_conversion(match_type);
        mongo::BSONObjBuilder term_builder;
        (this->*function)(field, vr, *or_it, term_builder);

        or_builder << term_builder.obj();
    }
    builder << "$or" << or_builder.arr();
}

GeneratorPACS
::GeneratorPACS():
    Generator(), _isconnected(false), _username("")
{
    // Nothing else.
}

GeneratorPACS
::~GeneratorPACS()
{
    // Nothing to do.
}

dcmtkpp::Value::Integer
GeneratorPACS
::initialize(dcmtkpp::Association const & association,
             dcmtkpp::message::Message const & message)
{
    // Get user identity
    this->_username = association.get_user_identity_primary_field();

    // Everything ok
    return dcmtkpp::message::Response::Success;
}

bool
GeneratorPACS
::done() const
{
    if (this->_cursor == NULL)
    {
        return true;
    }

    return !this->_cursor->more();
}

dcmtkpp::Value::Integer
GeneratorPACS
::initialize(mongo::BSONObj const & request)
{
    // Try to connect to database
    this->_isconnected = create_db_connection(this->_connection, this->_db_name);
    if (this->_isconnected == false)
    {
        return dcmtkpp::message::Response::ProcessingFailure;
    }

    // Everything ok
    return dcmtkpp::message::Response::Success;
}

void
GeneratorPACS
::set_username(const std::string &name)
{
    this->_username = name;
}

std::string
GeneratorPACS
::get_username() const
{
    return this->_username;
}

bool
GeneratorPACS
::is_connected() const
{
    return this->_isconnected;
}

GeneratorPACS::Match::Type
GeneratorPACS
::_get_match_type(std::string const & vr,
                  mongo::BSONElement const & element) const
{
    Match::Type type = Match::Unknown;

    if(element.isNull())
    {
        // C.2.2.2.3 Universal Matching
        // Value is zero-length
        type = Match::Universal;
    }
    else
    {
        bool const is_date_or_time = (vr == "DA" || vr == "DT" || vr == "TM");
        bool const has_wildcard_matching = (!is_date_or_time &&
            vr != "SL" && vr != "SS" && vr != "UL" &&
            vr != "FD" && vr != "FL" &&
            vr != "OB" && vr != "OF" && vr != "OW" && vr != "UN" &&
            vr != "DS" && vr != "US" &&
            vr != "UI");
        if(element.type() == mongo::String ||
           (element.type() == mongo::Object && vr == "PN"))
        {
            std::string value;
            if (element.type() == mongo::Object && vr == "PN")
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
                type = Match::SingleValue;
            }
            else if(has_wildcard_matching &&
                    value.find_first_of("?*") != std::string::npos)
            {
                // C.2.2.2.4 Wild Card Matching
                // Not a date, time, datetime, SL, SL, UL, US, FL, FD, OB,
                // OW, UN, AT, DS, IS, AS, UI and Value contains "*" or "?"
                type = Match::WildCard;
            }
            else if(is_date_or_time && value.find('-') != std::string::npos)
            {
                // C.2.2.2.5 Range Matching
                // Date, time or datetime, contains "-"
                type = Match::Range;
            }
        }
        else if (element.type() == mongo::NumberDouble ||
                 element.type() == mongo::NumberInt ||
                 element.type() == mongo::NumberLong)
        {
            type = Match::SingleValue;
        }
        else if(element.type() == mongo::Array && vr == "UI")
        {
            // C.2.2.2.2 List of UID Matching
            type = Match::ListOfUID;
        }
        else if(element.type() == mongo::Array && vr == "SQ")
        {
            // C.2.2.2.6 Sequence Matching
            type = Match::Sequence;
        }
        else if(element.type() == mongo::Array && vr != "SQ")
        {
            type = Match::MultipleValues;
        }
    }

    return type;
}

GeneratorPACS::DicomQueryToMongoQuery
GeneratorPACS
::_get_query_conversion(Match::Type const & match_type) const
{
    DicomQueryToMongoQuery function = NULL;
    if(match_type == Match::SingleValue)
    {
        function = &Self::_dicom_query_to_mongo_query<Match::SingleValue>;
    }
    else if(match_type == Match::ListOfUID)
    {
        function = &Self::_dicom_query_to_mongo_query<Match::ListOfUID>;
    }
    else if(match_type == Match::Universal)
    {
        function = &Self::_dicom_query_to_mongo_query<Match::Universal>;
    }
    else if(match_type == Match::WildCard)
    {
        function = &Self::_dicom_query_to_mongo_query<Match::WildCard>;
    }
    else if(match_type == Match::Range)
    {
        function = &Self::_dicom_query_to_mongo_query<Match::Range>;
    }
//    else if(match_type == Match::Sequence)
//    {
//        function = &Self::_dicom_query_to_mongo_query<Match::Sequence>;
//    }
    else if(match_type == Match::MultipleValues)
    {
        function = &Self::_dicom_query_to_mongo_query<Match::MultipleValues>;
    }
    else
    {
        function = &Self::_dicom_query_to_mongo_query<Match::Unknown>;
    }

    return function;
}

dcmtkpp::Value::Integer
GeneratorPACS
::_get_count(std::string const & relatedElement,
             std::string const & ofElement,
             std::string const & value)
{
    mongo::BSONObj const object = BSON("distinct" << "datasets" <<
                                       "key" << relatedElement <<
                                       "query" << BSON(ofElement << value));

    mongo::BSONObj info;
    bool ret = this->_connection.runCommand(this->_db_name,
                                            object, info);
    if (!ret)
    {
        // error
    }

    return info["values"].Array().size();
}

dcmtkpp::Element
GeneratorPACS
::compute_attribute(dcmtkpp::Tag const & tag, dcmtkpp::VR const & vr,
                    std::string const & value)
{
    if (tag == dcmtkpp::registry::InstanceAvailability) // Instance Availability
    {
        return dcmtkpp::Element({"ONLINE"}, vr);
    }
    else if (tag == dcmtkpp::registry::ModalitiesInStudy) // Modalities in Study
    {
        mongo::BSONObj const object = BSON("distinct" << "datasets" <<
                                           "key" << "00080060.Value" <<
                                           "query" << BSON("0020000d.Value" <<
                                                           value));

        mongo::BSONObj info;
        bool ret = this->_connection.runCommand(this->_db_name,
                                       object, info);
        if (!ret)
        {
            // error
        }

        dcmtkpp::Value::Strings values;
        for (auto const item : info.getField("values").Array())
        {
            values.push_back(item.String());
        }

        return dcmtkpp::Element(values, vr);
    }
    else if (tag == "00201200") // Number of Patient Related Study
    {
        auto size = this->_get_count("0020000d", "00100020.Value", value);
        return dcmtkpp::Element({size}, vr);
    }
    else if (tag == "00201202") // Number of Patient Related Series
    {
        auto size = this->_get_count("0020000e", "00100020.Value", value);
        return dcmtkpp::Element({size}, vr);
    }
    else if (tag == "00201204") // Number of Patient Related Instances
    {
        auto size = this->_get_count("00080018", "00100020.Value", value);
        return dcmtkpp::Element({size}, vr);
    }
    else if (tag == "00201206") // Number of Study Related Series
    {
        auto size = this->_get_count("0020000e", "0020000d.Value", value);
        return dcmtkpp::Element({size}, vr);
    }
    else if (tag == "00201208") // Number of Study Related Instances
    {
        auto size = this->_get_count("00080018", "0020000d.Value", value);
        return dcmtkpp::Element({size}, vr);
    }
    else if (tag == "00201209") // Number of Series Related Instances
    {
        auto size = this->_get_count("00080018", "0020000e.Value", value);
        return dcmtkpp::Element({size}, vr);
    }

    return dcmtkpp::Element();
}

} // namespace services

} // namespace dopamine
