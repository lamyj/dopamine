/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/


#include "dopamine/archive/DataSetGeneratorHelper.h"

#include <sstream>
#include <string>
#include <vector>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>

#include <odil/DataSet.h>
#include <odil/message/Response.h>
#include <odil/SCP.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/archive/mongo_query.h"
#include "dopamine/archive/Storage.h"

namespace dopamine
{

namespace archive
{

DataSetGeneratorHelper
::DataSetGeneratorHelper(
    mongo::DBClientConnection & connection, AccessControlList const & acl,
    std::string const & database, std::string const & bulk_database,
    std::string const & principal, std::string const & service)
: _connection(connection), _acl(acl),
  _storage(connection, database, bulk_database),
  _principal(principal), _service(service)
{
    // Nothing else
}

void
DataSetGeneratorHelper
::check_acl() const
{
    if(!this->_acl.is_allowed(this->_principal, this->_service))
    {
        std::ostringstream message;
        message
            << "User \"" << this->_principal << "\" "
            << "is not allowed to " << this->_service;

        odil::DataSet status_fields;
        status_fields.add(odil::registry::ErrorComment, { message.str() });

        throw odil::SCP::Exception(
            message.str(), odil::message::Response::RefusedNotAuthorized,
            status_fields);
    }
}

void
DataSetGeneratorHelper
::get_condition_and_projection(
    odil::DataSet const & data_set,
    mongo::BSONObjBuilder & condition_builder,
    mongo::BSONObjBuilder & projection_builder) const
{
    mongo::BSONArrayBuilder query_builder;
    as_mongo_query(data_set, query_builder, projection_builder);

    auto const query = query_builder.arr();
    auto const constraints = this->_acl.get_constraints(
        this->_principal, this->_service);

    mongo::BSONArrayBuilder condition_terms_builder;
    if(!query.isEmpty())
    {
        condition_terms_builder << BSON("$and" << query);
    }
    if(!constraints.isEmpty())
    {
        condition_terms_builder << constraints;
    }

    if(condition_terms_builder.arrSize()>0)
    {
        condition_builder << "$and" << condition_terms_builder.arr();
    }
    // Otherwise do nothing
}

void
DataSetGeneratorHelper
::set_results(std::vector<mongo::BSONObj> const & results)
{
    this->_results = results;
    this->_results_iterator = this->_results.begin();
}

bool
DataSetGeneratorHelper
::done() const
{
    return (this->_results_iterator == this->_results.end());
}

void
DataSetGeneratorHelper
::next()
{
    ++this->_results_iterator;
}

mongo::BSONObj const &
DataSetGeneratorHelper
::get() const
{
    return *this->_results_iterator;
}

unsigned int
DataSetGeneratorHelper
::count() const
{
    return this->_results.size();
}

void
DataSetGeneratorHelper
::store(odil::DataSet const & data_set)
{
    try
    {
        this->_storage.store(data_set);
    }
    catch(std::exception const & e)
    {
        odil::DataSet status;
        status.add(odil::registry::ErrorComment, { e.what()});
        throw odil::SCP::Exception(
            e.what(), odil::message::Response::ProcessingFailure, status);
    }
}

odil::DataSet
DataSetGeneratorHelper
::retrieve(std::string const & sop_instance_uid) const
{
    try
    {
        return this->_storage.retrieve(sop_instance_uid);
    }
    catch(std::exception const & e)
    {
        odil::DataSet status;
        status.add(odil::registry::ErrorComment, { e.what()});
        throw odil::SCP::Exception(
            e.what(), odil::message::Response::ProcessingFailure, status);
    }
}

} // namespace archive

} // namespace dopamine
