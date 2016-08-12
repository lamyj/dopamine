/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/archive/QueryDataSetGenerator.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>
#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/message/CFindRequest.h>
#include <odil/message/CFindResponse.h>
#include <odil/message/Response.h>
#include <odil/message/Request.h>
#include <odil/registry.h>
#include <odil/SCP.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/bson_converter.h"
#include "dopamine/logging.h"
#include "dopamine/utils.h"

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
    std::string const & database,
    odil::AssociationParameters const & parameters)
: _connection(connection), _acl(acl), _parameters(parameters),
  _helper(connection, acl, database, "", get_principal(parameters), "Query")
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

void
QueryDataSetGenerator
::initialize(odil::message::Request const & request)
{
    this->_helper.check_acl();

    odil::message::CFindRequest const find_request(request);
    this->_query = find_request.get_data_set();
    auto data_set = this->_query;

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

    mongo::BSONObjBuilder condition_builder;
    mongo::BSONObjBuilder projection_builder;
    this->_helper.get_condition_and_projection(
        data_set, condition_builder, projection_builder);
    auto const condition = condition_builder.obj();
    auto const projection = projection_builder.obj();

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
    std::vector<mongo::BSONObj> results;
    for(auto const & result: info["result"].Array())
    {
        results.push_back(result["_id"].Obj().getOwned());
    }

    this->_helper.set_results(results);

    DOPAMINE_LOG(DEBUG)
        << "Found " << results.size()
        << " matching entr" << (results.size()>1?"ies":"y");

    this->_dicom_data_set_up_to_date = false;
}

bool
QueryDataSetGenerator
::done() const
{
    if(this->_helper.done())
    {
        DOPAMINE_LOG(DEBUG) << "All matching entries have been sent";
    }
    return this->_helper.done();
}

void
QueryDataSetGenerator
::next()
{
    this->_helper.next();
    this->_dicom_data_set_up_to_date = false;
}

odil::DataSet
QueryDataSetGenerator
::get() const
{
    if(!this->_dicom_data_set_up_to_date)
    {
        this->_dicom_data_set = as_dataset(this->_helper.get());
        for(auto const & attribute: this->_additional_attributes)
        {
            auto const & function = this->_attribute_calculators.at(attribute);
            function(this, this->_dicom_data_set);
        }
        this->_dicom_data_set.add(
            odil::registry::SpecificCharacterSet, {"ISO_IR 192"});
        this->_dicom_data_set.add(
            odil::registry::QueryRetrieveLevel,
            this->_query[odil::registry::QueryRetrieveLevel]);
        this->_dicom_data_set.add(
            odil::registry::InstanceAvailability, {"ONLINE"});

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
            BSON(std::string(primary)+".Value" << data_set.as_string(primary, 0))
            << this->_acl.get_constraints(
                get_principal(this->_parameters), "Query")
           )
    ));
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
        odil::DataSet status;
        status.add(odil::registry::OffendingElement, {destination});
        status.add(odil::registry::ErrorComment, {info["errmsg"].String()});
        throw odil::SCP::Exception(
            info["errmsg"].String(),
            odil::message::CFindResponse::UnableToProcess, status);
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
            BSON(std::string(primary)+".Value" << data_set.as_string(primary, 0))
            << this->_acl.get_constraints(
                get_principal(this->_parameters), "Query")
        )
    ));
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
        odil::DataSet status;
        status.add(odil::registry::OffendingElement, {destination});
        status.add(odil::registry::ErrorComment, {info["errmsg"].String()});
        throw odil::SCP::Exception(
            info["errmsg"].String(),
            odil::message::CFindResponse::UnableToProcess, status);
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
