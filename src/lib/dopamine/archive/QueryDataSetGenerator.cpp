/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/archive/QueryDataSetGenerator.h"

#include <memory>
#include <string>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>
#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/message/CFindRequest.h>
#include <odil/message/Response.h>
#include <odil/message/Request.h>
#include <odil/registry.h>
#include <odil/SCP.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/archive/mongo_query.h"
#include "dopamine/bson_converter.h"

namespace dopamine
{

namespace archive
{

std::map<odil::Tag, QueryDataSetGenerator::AttributeCalculator> const
QueryDataSetGenerator
::_attribute_calculators=QueryDataSetGenerator::_create_attribute_calculators();

QueryDataSetGenerator
::QueryDataSetGenerator(
    mongo::DBClientConnection & connection, AccessControlList const & acl,
    std::string const & database)
: _connection(connection), _acl(acl)
{
    this->set_database(database);
}

QueryDataSetGenerator
::~QueryDataSetGenerator()
{
    // Nothing to do.
}

std::string const &
QueryDataSetGenerator
::get_database() const
{
    return this->_database;
}

void
QueryDataSetGenerator
::set_database(std::string const & database)
{
    this->_database = database;
    this->_namespace = database+".datasets";
}

std::string const &
QueryDataSetGenerator
::get_principal() const
{
    return this->_principal;
}

void
QueryDataSetGenerator
::set_principal(std::string const & principal)
{
    this->_principal = principal;
}

void
QueryDataSetGenerator
::set_principal(odil::AssociationParameters const & parameters)
{
    auto const identity = parameters.get_user_identity();
    if(identity.type == odil::AssociationParameters::UserIdentity::Type::None)
    {
        this->_principal = "";
    }
    else if(identity.type == odil::AssociationParameters::UserIdentity::Type::Username)
    {
        this->_principal = identity.primary_field;
    }
    else if(identity.type == odil::AssociationParameters::UserIdentity::Type::UsernameAndPassword)
    {
        this->_principal = identity.primary_field;
    }
    else
    {
        throw odil::SCP::Exception(
            "Cannot set principal", odil::message::Response::ProcessingFailure);
    }
}

void
QueryDataSetGenerator
::initialize(odil::message::Request const & request)
{
    if(!this->_acl.is_allowed(this->_principal, "Query"))
    {
        std::ostringstream message;
        message << "User \"" << this->_principal << "\" is not allowed to query";
        odil::DataSet status_fields;
        status_fields.add(odil::registry::ErrorComment, { message.str() });
        throw odil::SCP::Exception(
            message.str(), odil::message::Response::RefusedNotAuthorized,
            status_fields);
    }

    odil::message::CFindRequest const find_request(request);
    auto data_set = find_request.get_data_set();

    // Don't query additional attributes (PS 3.4, C.3.4), they are processed
    // later.
    this->_additional_attributes.clear();
    for(auto const it: this->_attribute_calculators)
    {
        auto const & tag = it.first;
        if(data_set.has(tag))
        {
            this->_additional_attributes.push_back(tag);
            data_set.remove(tag);
        }
    }

    mongo::BSONArrayBuilder query_builder;
    mongo::BSONObjBuilder projection_builder;
    as_mongo_query(data_set, query_builder, projection_builder);

    auto const query = query_builder.arr();
    auto const projection = projection_builder.obj();
    auto const constraints = this->_acl.get_constraints(
        this->_principal, "Query");

    mongo::BSONArrayBuilder condition_builder;
    if(!query.isEmpty())
    {
        condition_builder << BSON("$and" << query);
    }
    if(!constraints.isEmpty())
    {
        condition_builder << constraints;
    }
    mongo::BSONObj const condition(
        (condition_builder.arrSize()>0)
        ?BSON(
            "$and" << condition_builder.arr())
        :mongo::BSONObj());

    // Create the ID document for the group operator based on fields. This is
    // similar to a multi-key distinct.
    mongo::BSONObjBuilder group_id_builder;
    for(auto it=projection.begin(); it.more(); /* nothing */)
    {
        auto const element = it.next();
        std::string const field_name = element.fieldName();
        group_id_builder << field_name << ("$"+field_name);
    }
    auto const group_id(group_id_builder.obj());

    auto const pipeline = BSON_ARRAY(
        BSON("$match" << condition) <<
        BSON("$project" << projection) <<
        BSON("$group" << BSON("_id" << group_id))
    );

    mongo::BSONObj info;
    auto const ok = this->_connection.runCommand(
        this->_database,
        BSON("aggregate" << "datasets" << "pipeline" << pipeline), info);
    if(!ok)
    {
        throw odil::SCP::Exception(
            info["errmsg"].String(),
            odil::message::Response::ProcessingFailure);
    }

    // Make sure to get the full BSONObj: BSONElement does not own the data, it
    // only points to it, and the BSONObj itself may not own its data.
    auto const results = info["result"].Array();
    this->_results.resize(results.size());
    std::transform(
        results.begin(), results.end(), this->_results.begin(),
        [](mongo::BSONElement const & element) {
            return element["_id"].Obj().getOwned();
        });
    this->_results_iterator = this->_results.begin();
    this->_dicom_data_set_up_to_date = false;
}

bool
QueryDataSetGenerator
::done() const
{
    return (this->_results_iterator == this->_results.end());
}

void
QueryDataSetGenerator
::next()
{
    ++this->_results_iterator;
    this->_dicom_data_set_up_to_date = false;
}

odil::DataSet
QueryDataSetGenerator
::get() const
{
    if(!this->_dicom_data_set_up_to_date)
    {
        this->_dicom_data_set = as_dataset(*this->_results_iterator);
        for(auto const & attribute: this->_additional_attributes)
        {
            auto const & function = this->_attribute_calculators.at(attribute);
            function(this, this->_dicom_data_set);
        }

        this->_dicom_data_set_up_to_date = true;
    }
    return this->_dicom_data_set;
}

std::map<odil::Tag, QueryDataSetGenerator::AttributeCalculator>
QueryDataSetGenerator
::_create_attribute_calculators()
{
    return {
        {
            odil::registry::NumberOfPatientRelatedStudies,
            &QueryDataSetGenerator::_NumberOfPatientRelatedStudies
        },
        {
            odil::registry::NumberOfPatientRelatedSeries,
            &QueryDataSetGenerator::_NumberOfPatientRelatedSeries
        },
        {
            odil::registry::NumberOfPatientRelatedInstances,
            &QueryDataSetGenerator::_NumberOfPatientRelatedInstances
        },
        {
            odil::registry::NumberOfStudyRelatedSeries,
            &QueryDataSetGenerator::_NumberOfStudyRelatedSeries
        },
        {
            odil::registry::NumberOfStudyRelatedInstances,
            &QueryDataSetGenerator::_NumberOfStudyRelatedInstances
        },
        {
            odil::registry::NumberOfSeriesRelatedInstances,
            &QueryDataSetGenerator::_NumberOfSeriesRelatedInstances
        },
        {
            odil::registry::ModalitiesInStudy,
            &QueryDataSetGenerator::_ModalitiesInStudy
        },
        {
            odil::registry::SOPClassesInStudy,
            &QueryDataSetGenerator::_SOPClassesInStudy
        },
    };
}

void
QueryDataSetGenerator
::_xxx_in_yyy(
    odil::DataSet & data_set,
    odil::Tag const & primary, odil::Tag const & secondary,
    odil::Tag const & destination) const
{
    mongo::BSONObj const condition(BSON(
        "$and" << BSON_ARRAY(
            BSON(std::string(primary)+".Value" << data_set.as_string(primary, 0)) <<
            this->_acl.get_constraints(this->_principal, "Query"))));
    auto const projection = BSON(
        std::string(primary) << 1 << std::string(secondary) << 1);

    auto const pipeline = BSON_ARRAY(
        BSON("$match" << condition)
        << BSON("$project" << projection)
        << BSON(
            "$group" << BSON(
                "_id" << BSON(
                    std::string(primary) << "$"+std::string(primary)+".Value"
                )
                << "result" << BSON("$addToSet" << "$"+std::string(secondary)+".Value")
            )
        )
    );

    mongo::BSONObj info;
    auto const ok = this->_connection.runCommand(
        this->_database,
        BSON("aggregate" << "datasets" << "pipeline" << pipeline), info);
    if(!ok)
    {
        throw odil::SCP::Exception(
            info["errmsg"].String(),
            odil::message::Response::ProcessingFailure);
    }

    odil::Value::Strings values;
    auto const result = info["result"].Array()[0]["result"].Array();
    std::transform(
        result.begin(), result.end(), std::back_inserter(values),
        [](mongo::BSONElement const & element)
        {
            return element.Array()[0].String();
        });

    data_set.add(destination, values);
}
void
QueryDataSetGenerator
::_number_of_xxx_related_yyy(
    odil::DataSet & data_set,
    odil::Tag const & primary, odil::Tag const & secondary,
    odil::Tag const & destination) const
{
    mongo::BSONObj const condition(BSON(
        "$and" << BSON_ARRAY(
            BSON(std::string(primary)+".Value" << data_set.as_string(primary, 0)) <<
            this->_acl.get_constraints(this->_principal, "Query"))));
    auto const projection = BSON(
        std::string(primary) << 1 << std::string(secondary) << 1);

    auto const pipeline = BSON_ARRAY(
        BSON("$match" << condition)
        << BSON("$project" << projection)
        << BSON(
            "$group" << BSON(
                "_id" << BSON(
                    std::string(primary) << "$"+std::string(primary)+".Value" <<
                    std::string(secondary) << "$"+std::string(secondary)+".Value"
                )
            )
        )
    );

    mongo::BSONObj info;
    auto const ok = this->_connection.runCommand(
        this->_database,
        BSON("aggregate" << "datasets" << "pipeline" << pipeline), info);
    if(!ok)
    {
        throw odil::SCP::Exception(
            info["errmsg"].String(),
            odil::message::Response::ProcessingFailure);
    }

    data_set.add(
        destination, { odil::Value::Integer(info["result"].Array().size()) });
}

void
QueryDataSetGenerator
::_NumberOfPatientRelatedStudies(odil::DataSet & data_set) const
{
    this->_number_of_xxx_related_yyy(
        data_set,
        odil::registry::PatientID, odil::registry::StudyInstanceUID,
        odil::registry::NumberOfPatientRelatedStudies);
}

void
QueryDataSetGenerator
::_NumberOfPatientRelatedSeries(odil::DataSet & data_set) const
{
    this->_number_of_xxx_related_yyy(
        data_set,
        odil::registry::PatientID, odil::registry::SeriesInstanceUID,
        odil::registry::NumberOfPatientRelatedSeries);
}

void
QueryDataSetGenerator
::_NumberOfPatientRelatedInstances(odil::DataSet & data_set) const
{
    this->_number_of_xxx_related_yyy(
        data_set,
        odil::registry::PatientID, odil::registry::SOPInstanceUID,
        odil::registry::NumberOfPatientRelatedInstances);
}

void
QueryDataSetGenerator
::_NumberOfStudyRelatedSeries(odil::DataSet & data_set) const
{
    this->_number_of_xxx_related_yyy(
        data_set,
        odil::registry::StudyInstanceUID, odil::registry::SeriesInstanceUID,
        odil::registry::NumberOfStudyRelatedSeries);
}

void
QueryDataSetGenerator
::_NumberOfStudyRelatedInstances(odil::DataSet & data_set) const
{
    this->_number_of_xxx_related_yyy(
        data_set,
        odil::registry::StudyInstanceUID, odil::registry::SOPInstanceUID,
        odil::registry::NumberOfStudyRelatedInstances);
}

void
QueryDataSetGenerator
::_NumberOfSeriesRelatedInstances(odil::DataSet & data_set) const
{
    this->_number_of_xxx_related_yyy(
        data_set,
        odil::registry::SeriesInstanceUID, odil::registry::SOPInstanceUID,
        odil::registry::NumberOfSeriesRelatedInstances);
}


void
QueryDataSetGenerator
::_ModalitiesInStudy(odil::DataSet & data_set) const
{
    this->_xxx_in_yyy(
        data_set,
        odil::registry::StudyInstanceUID, odil::registry::Modality,
        odil::registry::ModalitiesInStudy);
}

void
QueryDataSetGenerator
::_SOPClassesInStudy(odil::DataSet & data_set) const
{
    this->_xxx_in_yyy(
        data_set,
        odil::registry::StudyInstanceUID, odil::registry::SOPClassUID,
        odil::registry::SOPClassesInStudy);
}
}

}
