/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/archive/GetDataSetGenerator.h"

#include <string>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/Exception.h>
#include <odil/message/CGetRequest.h>
#include <odil/message/Request.h>
#include <odil/MoveSCP.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/archive/DataSetGeneratorHelper.h"
#include "dopamine/utils.h"

namespace dopamine
{

namespace archive
{

GetDataSetGenerator
::GetDataSetGenerator(
    mongo::DBClientConnection & connection, AccessControlList const & acl,
    std::string const & database, std::string const & bulk_database,
    odil::AssociationParameters const & parameters)
: _connection(connection), _acl(acl), _parameters(parameters), _helper(acl)
{
    this->set_database(database);
    this->set_bulk_database(bulk_database);
    this->_helper.principal = get_principal(this->_parameters);
    this->_helper.service = "Retrieve";
}

GetDataSetGenerator
::~GetDataSetGenerator()
{
    // Nothing to do.
}

std::string const &
GetDataSetGenerator
::get_database() const
{
    return this->_database;
}

void
GetDataSetGenerator
::set_database(std::string const & database)
{
    this->_database = database;
    this->_namespace = database+".datasets";
}


std::string const &
GetDataSetGenerator
::get_bulk_database() const
{
    return this->_bulk_database;
}

void
GetDataSetGenerator
::set_bulk_database(std::string const & bulk_database)
{
    this->_bulk_database = bulk_database;
}

void
GetDataSetGenerator
::initialize(odil::message::Request const & request)
{
    this->_helper.check_acl();

    odil::message::CGetRequest const get_request(request);
    auto data_set = get_request.get_data_set();
    if(!data_set.has(odil::registry::SOPInstanceUID))
    {
        data_set.add(odil::registry::SOPInstanceUID);
    }

    mongo::BSONObjBuilder condition_builder;
    mongo::BSONObjBuilder projection_builder;
    this->_helper.get_condition_and_projection(
        data_set, condition_builder, projection_builder);

    auto const condition = condition_builder.obj();
    auto const projection = BSON(
        std::string(odil::registry::SOPInstanceUID) << 1
        << std::string(odil::registry::TransferSyntaxUID) << 1
        << "Content" << 1);

    auto const cursor = this->_connection.query(
        this->_namespace, condition, 0, 0, &projection);
    std::vector<mongo::BSONObj> results;
    while(cursor->more())
    {
        auto const object = cursor->next().getOwned();
        results.push_back(object);
    }
    this->_helper.set_results(results);

    this->_dicom_data_set_up_to_date = false;
}

bool
GetDataSetGenerator
::done() const
{
    return this->_helper.done();
}

void
GetDataSetGenerator
::next()
{
    this->_helper.next();
    this->_dicom_data_set_up_to_date = false;
}

odil::DataSet
GetDataSetGenerator
::get() const
{
    if(!this->_dicom_data_set_up_to_date)
    {
        this->_dicom_data_set = this->_helper.retrieve_data_set(
            this->_connection, this->_bulk_database);
        this->_dicom_data_set_up_to_date = true;
    }

    return this->_dicom_data_set;
}

unsigned int
GetDataSetGenerator
::count() const
{
    return this->_helper.count();
}

} // namespace archive

} // namespace dopamine
