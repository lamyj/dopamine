/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/archive/Storage.h"

#include <sstream>
#include <string>

#include <mongo/client/dbclient.h>
#include <odil/DataSet.h>
#include <odil/Reader.h>
#include <odil/registry.h>
#include <odil/VR.h>
#include <odil/Writer.h>

#include "dopamine/bson_converter.h"
#include "dopamine/Exception.h"

namespace dopamine
{

namespace archive
{

Storage
::Storage(
    mongo::DBClientConnection & connection,
    std::string const & database, std::string const & bulk_database)
: _connection(connection), _database(), _bulk_database(), _gridfs_limit(16000000)
{
    this->set_database(database);
    this->set_bulk_database(bulk_database);
}

std::string const &
Storage
::get_database() const
{
    return this->_database;
}

void
Storage
::set_database(std::string const & database)
{
    this->_database = database;
}

std::string const &
Storage
::get_bulk_database() const
{
    return this->_bulk_database;
}

void
Storage
::set_bulk_database(std::string const & bulk_database)
{
    this->_bulk_database = bulk_database;
}

unsigned int
Storage
::get_gridfs_limit() const
{
    return this->_gridfs_limit;
}

void
Storage
::set_gridfs_limit(unsigned int limit)
{
    this->_gridfs_limit = limit;
}

void
Storage
::store(odil::DataSet const & data_set)
{
    auto const & sop_instance_uid = data_set.as_string(
        odil::registry::SOPInstanceUID, 0);

    // Get the original binary content
    std::ostringstream content_stream;
    odil::Writer::write_file(data_set, content_stream);
    auto const content = content_stream.str();

    // Store the BSON data set minus private and binary fields
    odil::DataSet stored_data_set;
    for(auto const & item: data_set)
    {
        auto const & tag = item.first;
        // Skip private tags
        if(tag.group%2 == 1)
        {
            continue;
        }

        auto const & element = item.second;
        // Skip binary VRs
        if(odil::is_binary(element.vr))
        {
            continue;
        }

        stored_data_set.add(tag, element);
    }

    auto const bson_data_set = as_bson(stored_data_set);
    this->_connection.insert(this->_database+".datasets", bson_data_set);

    std::string error_message = this->_connection.getLastError(this->_database);
    if(!error_message.empty())
    {
        throw Exception("Could not store: "+error_message);
    }

    // Store the bulk data
    auto const database =
        this->_bulk_database.empty()?this->_database:this->_bulk_database;
    if(content_stream.tellp() > this->_gridfs_limit)
    {
        mongo::GridFS gridfs(this->_connection, database);
        auto const gridfs_object = gridfs.storeFile(
            content.c_str(), content.size(),
            data_set.as_string(odil::registry::SOPInstanceUID, 0));

        error_message = this->_connection.getLastError(database);
        if(error_message.empty())
        {
            // Add the reference to GridFS
            this->_connection.update(
                this->_database+".datasets",
                BSON(
                    std::string(odil::registry::SOPInstanceUID)+".Value"
                    << sop_instance_uid),
                BSON(
                    "$set" << BSON(
                        "Content"
                        << gridfs_object.getField("_id").OID().toString())));
        }
    }
    else if(this->_bulk_database.empty())
    {
        mongo::BSONObjBuilder builder;
        builder.appendBinData(
            "Content", content.size(), mongo::BinDataGeneral, content.c_str());
        this->_connection.update(
            this->_database+".datasets",
            BSON(
                std::string(odil::registry::SOPInstanceUID)+".Value"
                << sop_instance_uid),
            BSON("$set" << builder.obj()));
    }
    else
    {
        mongo::BSONObjBuilder builder;
        builder.genOID();
        builder << "SOPInstanceUID" << sop_instance_uid;
        builder.appendBinData(
            "Content", content.size(), mongo::BinDataGeneral, content.c_str());
        auto const bulk_object = builder.obj();
        this->_connection.insert(
            this->_bulk_database+".datasets", bulk_object);
        this->_connection.update(
            this->_database+".datasets",
            BSON(
                std::string(odil::registry::SOPInstanceUID)+".Value"
                << sop_instance_uid),
            BSON(
                "$set" << BSON(
                    "Content" << bulk_object["_id"].OID().toString())
            )
        );
    }

    error_message = this->_connection.getLastError(database);
    if(!error_message.empty())
    {
        this->_connection.remove(
            this->_database+".datasets",
            BSON(
                std::string(odil::registry::SOPInstanceUID)+".Value"
                << sop_instance_uid));
        throw Exception("Could not store: "+error_message);
    }
}

odil::DataSet
Storage
::retrieve(std::string const & sop_instance_uid) const
{
    mongo::BSONObj const fields(BSON("Content" << 1));
    auto const object = this->_connection.findOne(
        this->_database+".datasets",
        BSON(
            std::string(odil::registry::SOPInstanceUID)+".Value"
            << sop_instance_uid),
        &fields);
    if(object.isEmpty())
    {
        throw Exception("No such data set: "+sop_instance_uid);
    }

    auto const content = object.getField("Content");
    std::stringstream stream;

    if(content.type() == mongo::BSONType::String)
    {
        // Look in GridFS
        mongo::GridFS const gridfs(this->_connection, this->_database);
        auto file = gridfs.findFile(sop_instance_uid);
        if(!file.exists())
        {
            mongo::GridFS const gridfs(this->_connection, this->_bulk_database);
            file = gridfs.findFile(sop_instance_uid);
        }
        if(file.exists())
        {
            file.write(stream);
        }
        else
        {
            // Look in bulk data Content
            auto const bulk_data = this->_connection.findOne(
                this->_bulk_database+".datasets",
                BSON("SOPInstanceUID" << sop_instance_uid));
            if(bulk_data.isEmpty())
            {
                throw Exception(
                    "No such data set in bulk data: "+sop_instance_uid);
            }
            auto const bulk_content = bulk_data.getField("Content");
            int size=0;
            char const * begin = bulk_content.binDataClean(size);
            stream.write(begin, size);
        }
    }
    else if(content.type() == mongo::BSONType::BinData)
    {
        int size=0;
        char const * begin = content.binDataClean(size);
        stream.write(begin, size);
    }
    else
    {
        throw Exception("Unknown Content type: "+std::to_string(content.type()));
    }

    return odil::Reader::read_file(stream).second;
}

} // namespace archive

} // namespace dopamine
