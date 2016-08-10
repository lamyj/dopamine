/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/


#include "dopamine/archive/DataSetGeneratorHelper.h"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <mongo/bson/bson.h>
#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

#include <odil/DataSet.h>
#include <odil/message/Response.h>
#include <odil/Reader.h>
#include <odil/SCP.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/archive/mongo_query.h"
#include "dopamine/utils.h"

namespace dopamine
{

namespace archive
{

DataSetGeneratorHelper
::DataSetGeneratorHelper(AccessControlList const & acl)
: _acl(acl)
{
    // Nothing else
}

void
DataSetGeneratorHelper
::check_acl() const
{
    if(!this->_acl.is_allowed(this->principal, this->service))
    {
        std::ostringstream message;
        message
            << "User \"" << this->principal << "\" "
            << "is not allowed to " << this->service;

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
        this->principal, this->service);

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

odil::DataSet
DataSetGeneratorHelper
::retrieve_data_set(
    mongo::DBClientConnection & connection,
    std::string const & bulk_database) const
{
    auto const & result = this->get();
    // Need shared_ptr since istringstream has no affectation operator
    std::shared_ptr<std::stringstream> stream;

    auto const content = result.getField("Content");

    if(content.type() == mongo::BSONType::String)
    {
        auto const sop_instance_uid =
            result[std::string(odil::registry::SOPInstanceUID)]
                .Obj().getField("Value").Array()[0].String();

        mongo::GridFS gridfs(connection, bulk_database);
        auto const file = gridfs.findFile(sop_instance_uid);
        if(file.exists())
        {
            stream = std::make_shared<std::stringstream>();
            file.write(*stream);
        }
        else
        {
            auto const bulk_data = connection.findOne(
                bulk_database+".datasets",
                BSON("SOPInstanceUID" << sop_instance_uid));
            if(bulk_data.isEmpty())
            {
                throw odil::SCP::Exception(
                    "No bulk data",
                    odil::message::Response::NoSuchSOPInstance);
            }
            auto const bulk_data_content = bulk_data.getField("Content");
            int size=0;
            char const * begin = bulk_data_content.binDataClean(size);

            stream = std::make_shared<std::stringstream>(
                std::string(begin, size));
        }
    }
    else if(content.type() == mongo::BSONType::BinData)
    {
        int size=0;
        char const * begin = content.binDataClean(size);

        stream = std::make_shared<std::stringstream>(
            std::string(begin, size));
    }
    else
    {
        std::stringstream streamerror;
        streamerror << "Unknown type '" << content.type()
                    << "' for Content attribute";
        throw odil::SCP::Exception(
            "Unknown content type '"+std::to_string(content.type())+"'",
            odil::message::Response::ProcessingFailure);
    }

    return odil::Reader::read_file(*stream).second;
}

} // namespace archive

} // namespace dopamine
