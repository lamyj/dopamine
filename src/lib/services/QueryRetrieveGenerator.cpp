/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "QueryRetrieveGenerator.h"

#include <memory>
#include <string>

#include <mongo/client/dbclient.h>
#include <odil/AssociationParameters.h>
#include <odil/message/Request.h>
#include <odil/SCP.h>

#include "ConverterBSON/bson_converter.h"
#include "ConverterBSON/TagMatch.h"
#include "core/ExceptionPACS.h"
#include "dbconnection/MongoDBConnection.h"
#include "core/LoggerPACS.h"
#include "services/query_converter.h"

namespace dopamine
{

namespace services
{

QueryRetrieveGenerator
::QueryRetrieveGenerator(
    odil::AssociationParameters const & parameters,
    MongoDBConnection & db_connection)
: _db_connection(db_connection), _cursor(), _username("")
{
    // Get user identity
    auto const identity = parameters.get_user_identity();
    if(identity.type == odil::AssociationParameters::UserIdentity::Type::Kerberos ||
        identity.type == odil::AssociationParameters::UserIdentity::Type::SAML)
    {
        throw ExceptionPACS("Cannot get user name from identity");
    }
    this->_username = parameters.get_user_identity().primary_field;
}

QueryRetrieveGenerator
::~QueryRetrieveGenerator()
{
    if(!this->_cursor)
    {
        this->_cursor.release();
    }
}

bool
QueryRetrieveGenerator
::done() const
{
    return (this->_cursor == NULL) || !this->_cursor->more();
}

odil::DataSet
QueryRetrieveGenerator
::get() const
{
    return this->_data_set;
}

bool
QueryRetrieveGenerator
::_get_query_and_fields(
    odil::message::Request const & request,
    mongo::BSONArrayBuilder & query_builder,
    mongo::BSONObjBuilder & fields_builder) const
{
    bool const is_authorized = this->_db_connection.is_authorized(
        this->_username,
        odil::message::Message::Command::Type(request.get_command_field()));
    if(is_authorized)
    {
        logger_warning()
            << "User '" << this->_username
            << "' is not allowed to perform operation";
        return false;
    }

    auto const & data_set = request.get_data_set();

    if(!data_set.has(odil::registry::QueryRetrieveLevel))
    {
        throw odil::Exception("message::CFindResponse::MissingAttribute");
    }
    auto const query_retrieve_level = data_set.as_string(
        odil::registry::QueryRetrieveLevel, 0);

    // Query with DICOM syntax matches
    auto const dicom_query = as_bson(
        request.get_data_set(), FilterAction::INCLUDE,
        {
            {
                std::make_shared<converterBSON::TagMatch>(
                    odil::registry::SpecificCharacterSet),
                FilterAction::EXCLUDE
            },
            {
                std::make_shared<converterBSON::TagMatch>(
                    odil::registry::QueryRetrieveLevel),
                FilterAction::EXCLUDE
            }
        });

    // Build the MongoDB query and query fields from the DICOM query
    for(mongo::BSONObj::iterator it = dicom_query.begin(); it.more();)
    {
        auto const element = it.next();
        auto const object = element.Obj();

        auto const vr = object.getField("vr").String();
        std::string value_field = "Value";
        if(vr == "OB" || vr == "OD" || vr == "OF" || vr == "OL" || vr == "OW" ||
            vr == "UN")
        {
            value_field = "InlineBinary";
        }

        std::string field = std::string(element.fieldName()) + "." + value_field;
        if (vr == "PN")
        {
            field += ".Alphabetic";
        }

        auto const value = object.getField(value_field);
        auto const converter = get_query_converter(get_match_type(vr, value));

        // Convert the DICOM query term to MongoDB syntax
        mongo::BSONObjBuilder term;
        converter(field, vr, value, term);
        query_builder << term.obj();

        // Always include the query fields in the results
        fields_builder << element.fieldName() << 1;
    }

    // Make sure all mandatory fields are included
    std::vector<std::string> mandatory_fields = {
        odil::registry::SpecificCharacterSet,
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
        if(!fields_builder.hasField(field))
        {
            fields_builder << field << 1;
        }
    }

    return true;
}

} // namespace services

} // namespace dopamine
