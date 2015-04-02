/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "ConverterBSON/BSONToDataSet.h"
#include "ConverterBSON/DataSetToBSON.h"
#include "ConverterBSON/TagMatch.h"
#include "core/LoggerPACS.h"
#include "ResponseGenerator.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace services
{

// Define Unknown specialization first, since other specializations use it.
template<>
void
ResponseGenerator
::_dicom_query_to_mongo_query<ResponseGenerator::Match::Unknown>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    // Default action: convert to string
    builder << field << value.String();
}

template<>
void
ResponseGenerator
::_dicom_query_to_mongo_query<ResponseGenerator::Match::SingleValue>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    builder << field << value.String();
}

template<>
void
ResponseGenerator
::_dicom_query_to_mongo_query<ResponseGenerator::Match::ListOfUID>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    mongo::BSONArrayBuilder or_builder;
    std::vector<mongo::BSONElement> or_terms = value.Array();
    for(std::vector<mongo::BSONElement>::const_iterator or_it=or_terms.begin();
        or_it!=or_terms.end(); ++or_it)
    {
        or_builder << BSON(field << (*or_it));
    }
    builder << "$or" << or_builder.arr();
}

template<>
void
ResponseGenerator
::_dicom_query_to_mongo_query<ResponseGenerator::Match::Universal>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    // Universal is not part of the MongoDB query : do nothing
}

template<>
void
ResponseGenerator
::_dicom_query_to_mongo_query<ResponseGenerator::Match::WildCard>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    // Convert DICOM regex to PCRE: replace "*" by ".*", "?" by ".",
    // and escape other special PCRE characters (these are :
    // \^$.[]()+{}
    //
    std::string regex = value.String();
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
ResponseGenerator
::_dicom_query_to_mongo_query<ResponseGenerator::Match::Range>(
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
ResponseGenerator
::_dicom_query_to_mongo_query<ResponseGenerator::Match::MultipleValues>(
    std::string const & field, std::string const & vr,
    mongo::BSONElement const & value,
    mongo::BSONObjBuilder & builder) const
{
    mongo::BSONArrayBuilder or_builder;
    std::vector<mongo::BSONElement> or_terms = value.Array();
    for(std::vector<mongo::BSONElement>::const_iterator or_it=or_terms.begin();
        or_it!=or_terms.end(); ++or_it)
    {
        Match::Type const match_type = this->_get_match_type(vr, *or_it);

        DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);
        mongo::BSONObjBuilder term_builder;
        (this->*function)(field, vr, *or_it, term_builder);

        or_builder << term_builder.obj();
    }
    builder << "$or" << or_builder.arr();
}

ResponseGenerator
::ResponseGenerator(T_ASC_Association *request_association,
                    const std::string &service_name):
    _request_association(request_association), _service_name(service_name),
    _ourAETitle(""), _status(STATUS_Pending), _query_retrieve_level(""),
    _convert_modalities_in_study(false)
{
    // get AE title from association
    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(this->_request_association->params, NULL, aeTitle, NULL);
    _ourAETitle = std::string(aeTitle);

    // Create DataBase Connection
    create_db_connection(this->_connection, this->_db_name);
}

ResponseGenerator
::~ResponseGenerator()
{
    // Nothing to do
}

void ResponseGenerator::cancel()
{
    loggerWarning() << "Function Not implemented: ResponseGenerator::cancel()";
}

Uint16
ResponseGenerator
::set_query(DcmDataset *dataset)
{
    if (this->_connection.isFailed())
    {
        loggerWarning() << "Could not connect to database: " << this->_db_name;
        if (this->_service_name == Service_Query)
        {
            return 0xa700; // STATUS_FIND_Refused_OutOfResources
        }
        return 0xa701; // STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches or
                       // STATUS_GET_Refused_OutOfResourcesNumberOfMatches
    }

    // Look for user authorization
    std::string const username =
            get_username(this->_request_association->params->DULparams.reqUserIdentNeg);
    if ( ! is_authorized(this->_connection, this->_db_name, username, this->_service_name) )
    {
        loggerWarning() << "User '" << username << "' not allowed to perform "
                        << this->_service_name;
        if (this->_service_name == Service_Query)
        {
            return 0xa700; // STATUS_FIND_Refused_OutOfResources
        }
        return 0xa701; // STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches or
                       // STATUS_GET_Refused_OutOfResourcesNumberOfMatches
    }

    mongo::BSONObj constraint = get_constraint_for_user(this->_connection,
                                                        this->_db_name,
                                                        username,
                                                        this->_service_name);

    // Convert the dataset to BSON, excluding Query/Retrieve Level.
    DataSetToBSON dataset_to_bson;

    dataset_to_bson.get_filters().push_back(
        std::make_pair(TagMatch::New(DCM_QueryRetrieveLevel),
                       DataSetToBSON::FilterAction::EXCLUDE));
    dataset_to_bson.get_filters().push_back(
        std::make_pair(TagMatch::New(DCM_SpecificCharacterSet),
                       DataSetToBSON::FilterAction::EXCLUDE));
    dataset_to_bson.set_default_filter(DataSetToBSON::FilterAction::INCLUDE);

    mongo::BSONObjBuilder query_builder;
    dataset_to_bson(dataset, query_builder);
    mongo::BSONObj const query_dataset = query_builder.obj();

    // Build the MongoDB query and query fields from the query dataset.
    mongo::BSONObjBuilder db_query;
    mongo::BSONObjBuilder fields_builder;
    for(mongo::BSONObj::iterator it=query_dataset.begin(); it.more();)
    {
        mongo::BSONElement const element = it.next();
        std::vector<mongo::BSONElement> const array = element.Array();

        // Always include the field in the results
        fields_builder << element.fieldName() << 1;

        std::string const vr = array[0].String();
        mongo::BSONElement const & value = array[1];
        Match::Type const match_type = this->_get_match_type(vr, value);

        DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);
        // Match the array element containing the value
        (this->*function)(std::string(element.fieldName())+".1", vr, value, db_query);
    }

    if (this->_service_name != Service_Query)
    {
        // retrieve 'location' field
        fields_builder << "location" << 1;
    }

    // Always include Specific Character Set in results.
    if(!fields_builder.hasField("00080005"))
    {
        fields_builder << "00080005" << 1;
    }

    // Always include the keys for the query level and its higher levels
    OFString ofstring;
    OFCondition condition = dataset->findAndGetOFString(DCM_QueryRetrieveLevel,
                                                        ofstring);
    if (condition.bad())
    {
        dopamine::loggerError() << "Cannot find DCM_QueryRetrieveLevel: "
                                << condition .text();
        return 0xa900; // STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass or
                       // STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass or
                       // STATUS_GET_Failed_IdentifierDoesNotMatchSOPClass
    }

    this->_query_retrieve_level = std::string(ofstring.c_str());
    if(!fields_builder.hasField("00100020"))
    {
        fields_builder << "00100020" << 1;
    }
    if((this->_query_retrieve_level=="STUDY" ||
        this->_query_retrieve_level=="SERIES" ||
        this->_query_retrieve_level=="IMAGE") && !fields_builder.hasField("0020000d"))
    {
        fields_builder << "0020000d" << 1;
    }
    if((this->_query_retrieve_level=="SERIES" ||
        this->_query_retrieve_level=="IMAGE") && !fields_builder.hasField("0020000e"))
    {
        fields_builder << "0020000e" << 1;
    }
    if(this->_query_retrieve_level=="IMAGE" && !fields_builder.hasField("00080018"))
    {
        fields_builder << "00080018" << 1;
    }

    // Handle reduce-related attributes
    std::string reduce_function;
    mongo::BSONObjBuilder initial_builder;

    // Number of XXX Related Instances (0020,120X)
    if(query_dataset.hasField("00201204"))
    {
        this->_instance_count_tag = DCM_NumberOfPatientRelatedInstances;
    }
    else if(query_dataset.hasField("00201208"))
    {
        this->_instance_count_tag = DCM_NumberOfStudyRelatedInstances;
    }
    else if(query_dataset.hasField("00201209"))
    {
        this->_instance_count_tag = DCM_NumberOfSeriesRelatedInstances;
    }
    else
    {
        this->_instance_count_tag = DCM_UndefinedTagKey;
    }
    if (this->_instance_count_tag != DCM_UndefinedTagKey)
    {
        reduce_function += "result.instance_count+=1;";
        initial_builder << "instance_count" << 0;
    }

    // Modalities in Study (0008,0061)
    if(this->_service_name == Service_Query && query_dataset.hasField("00080061"))
    {
        // Use the Modality attribute
        mongo::BSONElement modalities = query_dataset["00080061"];
        Match::Type const match_type =
            this->_get_match_type("CS", modalities);
        DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);
        (this->*function)("00080060.1", "CS", modalities, db_query);
        fields_builder << "00080060" << 1;
        reduce_function +=
            "if(result.modalities_in_study.indexOf(current[\"00080060\"][1])==-1) "
            "{ result.modalities_in_study.push(current[\"00080060\"][1]); }";
        initial_builder << "modalities_in_study" << mongo::BSONArrayBuilder().arr();
        this->_convert_modalities_in_study = true;
    }
    else
    {
        this->_convert_modalities_in_study = false;
    }

    mongo::BSONArrayBuilder finalquerybuilder;
    finalquerybuilder << constraint << db_query.obj();
    mongo::BSONObjBuilder finalquery;
    finalquery << "$and" << finalquerybuilder.arr();

    // Format the reduce function
    reduce_function = "function(current, result) { " + reduce_function + " }";

    // Perform the DB query.
    mongo::BSONObj const fields = fields_builder.obj();
    mongo::BSONObj group_command = BSON("group" << BSON(
        "ns" << "datasets" << "key" << fields << "cond" << finalquery.obj() <<
        "$reduce" << reduce_function << "initial" << initial_builder.obj()
    ));

    this->_cursor = this->_connection.query(this->_db_name, group_command);

    return STATUS_Pending;
}

ResponseGenerator::Match::Type
ResponseGenerator
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
            vr != "SL" && vr != "SS" && vr != "UL" && vr != "UL" &&
            vr != "FD" && vr != "FL" &&
            vr != "OB" && vr != "OF" && vr != "OW" && vr != "UN" &&
            vr != "DS" && vr != "US" &&
            vr != "UI");
        if(element.type() == mongo::String)
        {
            std::string value(element);
            // Not a date or time, no wildcard AND date or time, no range
            if(!(!is_date_or_time && value.find_first_of("?*") != std::string::npos) &&
               !(is_date_or_time && value.find('-') != std::string::npos))
            {
                // C.2.2.2.1 Single Value Matching
                // Non-zero length AND
                //   not a date or time or datetime, contains no wildcard OR
                //   a date or time or datetime, contains a single date or time
                //   or datetime with not "-"
                type = Match::SingleValue;
            }
            else if(has_wildcard_matching && value.find_first_of("?*") != std::string::npos)
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

ResponseGenerator::DicomQueryToMongoQuery
ResponseGenerator
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

} // namespace services

} // namespace dopamine
