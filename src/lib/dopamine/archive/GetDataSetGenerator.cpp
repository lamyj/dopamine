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
#include "dopamine/logging.h"
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
: _connection(connection), _acl(acl), _parameters(parameters),
  _helper(
    connection, acl, database, bulk_database, get_principal(parameters),
    "Retrieve")
{
    this->_namespace = database+".datasets";
}

GetDataSetGenerator
::~GetDataSetGenerator()
{
    // Nothing to do.
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
    DOPAMINE_LOG(DEBUG)
        << "Sending " << results.size()
        << " instance" << (results.size()>1?"s":"");

    this->_dicom_data_set_up_to_date = false;
}

bool
GetDataSetGenerator
::done() const
{
    if(this->_helper.done())
    {
        DOPAMINE_LOG(DEBUG) << "All matching entries have been sent";
    }
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
        auto const current = this->_helper.get();
        auto const sop_instance_uid = current[
            std::string(odil::registry::SOPInstanceUID)][
                "Value"].Array()[0].String();
        this->_dicom_data_set = this->_helper.retrieve(sop_instance_uid);
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
