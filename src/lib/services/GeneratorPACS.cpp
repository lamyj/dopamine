/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <odil/message/Response.h>

#include "core/ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "GeneratorPACS.h"

namespace dopamine
{

namespace services
{

template<>
void
GeneratorPACS
::_add_value_to_builder<int32_t>(mongo::BSONObjBuilder &builder,
                                std::string const & field,
                                std::string const & value) const
{
    // Fix compilation error for i386
    builder.appendIntOrLL(field, boost::lexical_cast<int32_t>(value));
}

template<>
void
GeneratorPACS
::_add_value_to_builder<uint32_t>(mongo::BSONObjBuilder &builder,
                                std::string const & field,
                                std::string const & value) const
{
    // Fix compilation error for i386
    builder.appendIntOrLL(field, boost::lexical_cast<uint32_t>(value));
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

    if      (vr == "DS")
    {
        this->_add_value_to_builder<double>(
                    builder, field, MongoDBConnection::as_string(value));
    }
    else if (vr == "FD")
    {
        this->_add_value_to_builder<double>(
                    builder, field, MongoDBConnection::as_string(value));
    }
    else if (vr == "FL")
    {
        this->_add_value_to_builder<float>(
                    builder, field, MongoDBConnection::as_string(value));
    }
    else if (vr == "IS")
    {
        this->_add_value_to_builder<int32_t>(
                    builder, field, MongoDBConnection::as_string(value));
    }
    else if (vr == "SL")
    {
        this->_add_value_to_builder<int32_t>(
                    builder, field, MongoDBConnection::as_string(value));
    }
    else if (vr == "SS")
    {
        this->_add_value_to_builder<int16_t>(
                    builder, field, MongoDBConnection::as_string(value));
    }
    else if (vr == "UL")
    {
        this->_add_value_to_builder<uint32_t>(
                    builder, field, MongoDBConnection::as_string(value));
    }
    else if (vr == "US")
    {
        this->_add_value_to_builder<uint16_t>(
                    builder, field, MongoDBConnection::as_string(value));
    }
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
    Generator(), _connection(NULL), _isconnected(false), _username(""),
    _query_retrieve_level(""), _instance_count_tags({}), _include_fields({}),
    _maximum_results(0), _skipped_results(0)
{
    // Nothing else.
}

GeneratorPACS
::~GeneratorPACS()
{
    if (this->_connection != NULL)
    {
        this->_cursor.release();
        delete this->_connection;
    }
}

odil::Value::Integer
GeneratorPACS
::initialize(odil::Association const & association,
             odil::message::Message const & message)
{
    // Get user identity
    this->_username =
        association.get_parameters().get_user_identity().primary_field;

    // Everything ok
    return odil::message::Response::Success;
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

odil::Value::Integer
GeneratorPACS
::initialize(mongo::BSONObj const & request)
{
    // Get configuration for Database connection
    MongoDBInformation db_information;
    std::string db_host = "";
    int db_port = -1;
    std::vector<std::string> indexeslist;
    ConfigurationPACS::get_instance().get_database_configuration(db_information,
                                                                 db_host,
                                                                 db_port,
                                                                 indexeslist);

    // Create connection with Database
    this->_connection = new MongoDBConnection(db_information, db_host, db_port,
                                              indexeslist);

    // Try to connect
    this->_isconnected = this->_connection->connect();
    if (this->_isconnected == false)
    {
        return odil::message::Response::ProcessingFailure;
    }

    // Everything ok
    return odil::message::Response::Success;
}

std::pair<std::string, int>
GeneratorPACS
::get_peer_information(std::string const & ae_title)
{
    return this->_connection->get_peer_information(ae_title);
}

void
GeneratorPACS
::set_username(std::string const & name)
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

void
GeneratorPACS
::set_query_retrieve_level(std::string const & query_retrieve_level)
{
    this->_query_retrieve_level = query_retrieve_level;
}

std::string
GeneratorPACS
::get_query_retrieve_level() const
{
    return this->_query_retrieve_level;
}

std::vector<std::string>
GeneratorPACS
::get_instance_count_tags() const
{
    return this->_instance_count_tags;
}

void
GeneratorPACS
::set_include_fields(std::vector<std::string> const & include_fields)
{
    this->_include_fields = include_fields;
}

std::vector<std::string> &
GeneratorPACS
::get_include_fields()
{
    return this->_include_fields;
}

void
GeneratorPACS
::set_maximum_results(int maximum_results)
{
    this->_maximum_results = maximum_results;
}

int
GeneratorPACS
::get_maximum_results() const
{
    return this->_maximum_results;
}

void
GeneratorPACS
::set_skipped_results(int skipped_results)
{
    this->_skipped_results = skipped_results;
}

int
GeneratorPACS
::get_skipped_results() const
{
    return this->_skipped_results;
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

bool
GeneratorPACS
::extract_query_retrieve_level(mongo::BSONObj const & mongo_object)
{
    // Query retrieve level should be present
    if (!mongo_object.hasField("00080052"))
    {
        logger_warning() << "Cannot find field QueryRetrieveLevel";
        return false;
    }

    // Read the Query Retrieve Level
    mongo::BSONObj const field_00080052 =
            mongo_object.getField("00080052").Obj();
    this->_query_retrieve_level =
            field_00080052.getField("Value").Array()[0].String();

    return true;
}

odil::Value::Integer
GeneratorPACS
::_get_count(std::string const & relatedElement,
             std::string const & ofElement,
             std::string const & value)
{
    mongo::BSONObj const object = BSON("distinct" << "datasets" <<
                                       "key" << relatedElement <<
                                       "query" << BSON(ofElement << value));

    mongo::BSONObj info;
    bool ret = this->_connection->run_command(object, info);
    if (!ret)
    {
        // error
    }

    return info["values"].Array().size();
}

odil::Element
GeneratorPACS
::compute_attribute(odil::Tag const & tag, odil::VR const & vr,
                    std::string const & value)
{
    if (tag == odil::registry::InstanceAvailability) // Instance Availability
    {
        return odil::Element({"ONLINE"}, vr);
    }
    else if (tag == odil::registry::ModalitiesInStudy) // Modalities in Study
    {
        mongo::BSONObj const object = BSON("distinct" << "datasets" <<
                                           "key" << "00080060.Value" <<
                                           "query" << BSON("0020000d.Value" <<
                                                           value));

        mongo::BSONObj info;
        bool ret = this->_connection->run_command(object, info);
        if (!ret)
        {
            // error
        }

        odil::Value::Strings values;
        for (auto const item : info.getField("values").Array())
        {
            values.push_back(item.String());
        }

        return odil::Element(values, vr);
    }
    else if (tag == "00201200") // Number of Patient Related Study
    {
        auto size = this->_get_count("0020000d", "00100020.Value", value);
        return odil::Element({size}, vr);
    }
    else if (tag == "00201202") // Number of Patient Related Series
    {
        auto size = this->_get_count("0020000e", "00100020.Value", value);
        return odil::Element({size}, vr);
    }
    else if (tag == "00201204") // Number of Patient Related Instances
    {
        auto size = this->_get_count("00080018", "00100020.Value", value);
        return odil::Element({size}, vr);
    }
    else if (tag == "00201206") // Number of Study Related Series
    {
        auto size = this->_get_count("0020000e", "0020000d.Value", value);
        return odil::Element({size}, vr);
    }
    else if (tag == "00201208") // Number of Study Related Instances
    {
        auto size = this->_get_count("00080018", "0020000d.Value", value);
        return odil::Element({size}, vr);
    }
    else if (tag == "00201209") // Number of Series Related Instances
    {
        auto size = this->_get_count("00080018", "0020000e.Value", value);
        return odil::Element({size}, vr);
    }

    return odil::Element();
}

std::string
GeneratorPACS
::replace(std::string const & value,
          std::string const & old,
          std::string const & new_)
{
    std::string result(value);
    size_t begin=0;
    while(std::string::npos != (begin=result.find(old, begin)))
    {
        result = result.replace(begin, old.size(), new_);
        begin = (begin+new_.size()<result.size())?begin+new_.size()
                                                 :std::string::npos;
    }

    return result;
}

} // namespace services

} // namespace dopamine
