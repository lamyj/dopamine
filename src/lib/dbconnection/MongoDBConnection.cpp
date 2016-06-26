/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include <boost/regex.hpp>

#include <odil/message/CStoreResponse.h>
#include <odil/Exception.h>
#include <odil/Reader.h>
#include <odil/Tag.h>
#include <odil/Writer.h>

// Insert dbclient before gridfs
#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

#include "ConverterBSON/bson_converter.h"
#include "ConverterBSON/IsPrivateTag.h"
#include "ConverterBSON/VRMatch.h"
#include "core/LoggerPACS.h"
#include "MongoDBConnection.h"

namespace dopamine
{

MongoDBConnection
::MongoDBConnection(MongoDBInformation const & db_information,
                    std::string const & host_name,
                    int port,
                    std::vector<std::string> const & indexes)
    : _database_information(db_information), _host_name(host_name),
      _port(port), _indexes(indexes)
{
    // Nothing else.
}

MongoDBConnection
::~MongoDBConnection()
{
    // Nothing to do.
}

mongo::DBClientConnection const &
dopamine
::MongoDBConnection::get_connection() const
{
    return this->_database_information.connection;
}

mongo::DBClientConnection &
MongoDBConnection
::get_connection()
{
    return this->_database_information.connection;
}


std::string const &
MongoDBConnection
::get_db_name() const
{
    return this->_database_information.get_db_name();
}

void
MongoDBConnection
::set_db_name(std::string const & db_name)
{
    this->_database_information.set_db_name(db_name);
}

std::string const &
MongoDBConnection
::get_bulk_data_db() const
{
    return this->_database_information.get_bulk_data();
}

std::string
MongoDBConnection
::get_host_name() const
{
    return this->_host_name;
}

void
MongoDBConnection
::set_host_name(std::string const & host_name)
{
    this->_host_name = host_name;
}

int
MongoDBConnection
::get_port() const
{
    return this->_port;
}

void
MongoDBConnection
::set_port(int port)
{
    this->_port = port;
}

std::vector<std::string>
MongoDBConnection
::get_indexes() const
{
    return this->_indexes;
}

void
MongoDBConnection
::set_indexes(std::vector<std::string> const & indexes)
{
    this->_indexes = indexes;
}

bool
MongoDBConnection
::connect()
{
    // Check configuration
    if (this->get_db_name() == "" ||
        this->get_host_name() == "" ||
        this->get_port() < 0)
    {
        return false;
    }

    // Try to connect database
    // Disconnect is automatic when it calls the destructors
    std::string errormsg = "";
    if ( ! this->_database_information.connection.connect(
             mongo::HostAndPort(this->get_host_name(), this->get_port()),
                                errormsg) )
    {
        logger_error() << errormsg;
        return false;
    }

    /*/ Database authentication
    if (!this->_database_information.connection.auth(
                this->get_db_name(), this->_database_information.get_user(),
                this->_database_information.get_password(), errormsg))
    {
        logger_error() << errormsg;
        return false;
    }

    // Bulk Database authentication
    if (this->get_db_name() != this->get_bulk_data_db() &&
        !this->_database_information.connection.auth(
            this->get_bulk_data_db(), this->_database_information.get_user(),
            this->_database_information.get_password(), errormsg))
    {
        logger_error() << errormsg;
        return false;
    }*/

    // Create indexes
    std::string const datasets_table = this->get_db_name() + ".datasets";

    for (auto currentIndex : this->get_indexes())
    {
        try
        {
            odil::Tag currenttag(currentIndex);

            std::stringstream stream;
            stream << "\"" << std::string(currenttag) << "\"";

            this->_database_information.connection.ensureIndex
                (
                    datasets_table,
                    BSON(stream.str() << 1),
                    false,
                    currenttag.get_name()
                );
        }
        catch (odil::Exception const & exc)
        {
            logger_warning() << "Ignore index '" << currentIndex
                             << "', reason: " << exc.what();
        }
    }

    if(this->get_db_name() != this->get_bulk_data_db())
    {
        this->_database_information.connection.ensureIndex(
                    this->get_bulk_data_db()+".datasets",
                    BSON("SOPInstanceUID" << 1));
    }

    return true;
}

bool
MongoDBConnection
::is_authorized(std::string const & user,
                odil::message::Message::Command::Type command)
{
    mongo::BSONArrayBuilder builder;
    if (user != "")
    {
        builder << "*";
    }
    builder << user;

    std::string const servicename = this->as_string(command);

    mongo::BSONObjBuilder fields_builder;
    fields_builder << "principal_name" << BSON("$in" << builder.arr())
                   << "service" << BSON("$in" << BSON_ARRAY("*" <<
                                                            servicename));

    mongo::BSONObj const group_command = BSON("count" << "authorization" <<
                                              "query" << fields_builder.obj());

    mongo::BSONObj info;
    this->_database_information.connection.runCommand(this->get_db_name(), group_command, info, 0);

    // If the command correctly executed and database entries match
    if (info["ok"].Double() == 1 && info["n"].Double() > 0)
    {
        return true;
    }

    // Not allowed
    return false;
}

mongo::BSONObj
MongoDBConnection
::get_constraints(std::string const & user,
                  odil::message::Message::Command::Type command)
{
    mongo::BSONArrayBuilder builder;
    if (user != "")
    {
        builder << "*";
    }
    builder << user;

    std::string const servicename = this->as_string(command);

    // Create Query with user's authorization
    mongo::BSONObjBuilder fields_builder;
    fields_builder << "principal_name" << BSON("$in" << builder.arr())
                   << "service" << BSON("$in" << BSON_ARRAY("*" <<
                                                            servicename));
    mongo::BSONObjBuilder initial_builder;
    mongo::BSONObj const group_command =
            BSON("group" << BSON("ns" << "authorization" <<
                                 "key" << BSON("dataset" << 1) <<
                                 "cond" << fields_builder.obj() <<
                                 "$reduce" << "function(current, result) { }" <<
                                 "initial" << initial_builder.obj()
    ));

    mongo::BSONObj result;
    this->_database_information.connection.runCommand(this->get_db_name(), group_command, result, 0);

    if (result["ok"].Double() != 1 || result["count"].Double() == 0)
    {
        // Create a constraint to deny all actions
        mongo::BSONObjBuilder builder_none;
        builder_none << "00080018.Value"
                     << BSON_ARRAY("_db8eeea6_e0eb_48b8_9a02_a94926b76992");

        return builder_none.obj();
    }

    mongo::BSONArrayBuilder constaintarray;
    for (auto item : result["retval"].Array())
    {
        // if dataset: "" or dataset: {} => all is allowed
        if ((item["dataset"].type() == mongo::BSONType::String &&
             item["dataset"].String() == "") ||
            (item["dataset"].type() == mongo::BSONType::Object &&
             item["dataset"].Obj().isEmpty()))
        {
            // No constraint
            return mongo::BSONObj();
        }

        /*
         * Warning: throw an exception if
         * item["dataset"].type() != mongo::BSONType::Object
         */

        mongo::BSONArrayBuilder andarray;
        for(mongo::BSONObj::iterator it = item["dataset"].Obj().begin();
            it.more();)
        {
            mongo::BSONElement const element = it.next();

            mongo::BSONObjBuilder object;
            if (element.type() == mongo::BSONType::RegEx)
            {
                object.appendRegex(std::string(element.fieldName())+".Value",
                                   element.regex(), "");
            }
            else
            {
                object << std::string(element.fieldName())+".Value"
                       << as_string(element);
            }
            andarray << object.obj();
        }
        mongo::BSONObjBuilder andobject;
        andobject << "$and" << andarray.arr();
        constaintarray << andobject.obj();
    }
    mongo::BSONObjBuilder constraint;
    constraint << "$or" << constaintarray.arr();

    return constraint.obj();
}

mongo::unique_ptr<mongo::DBClientCursor>
MongoDBConnection
::get_datasets_cursor(mongo::Query const & query, int nToReturn,
                      int nToSkip, mongo::BSONObj const & fieldsToReturn)
{
    return this->_database_information.connection.query(this->get_db_name() + ".datasets", query,
                                   nToReturn, nToSkip, &fieldsToReturn);
}

bool MongoDBConnection::run_command(mongo::BSONObj const & command, mongo::BSONObj & response)
{
    bool ret = this->_database_information.connection.runCommand(this->get_db_name(),
                                            command, response);

    // If an error occurred
    if (!ret || response["ok"].Double() != 1)
    {
        return false;
    }

    return true;
}

odil::Value::Integer
MongoDBConnection
::insert_dataset(std::string const & username,
                 odil::DataSet const & dataset,
                 std::string const & callingaet)
{
    Filters filters = {};
    filters.push_back(std::make_pair(
                          converterBSON::IsPrivateTag::New(),
                          FilterAction::EXCLUDE));
    filters.push_back(std::make_pair(
                          converterBSON::VRMatch::New(odil::VR::OB),
                          FilterAction::EXCLUDE));
    filters.push_back(std::make_pair(
                          converterBSON::VRMatch::New(odil::VR::OF),
                          FilterAction::EXCLUDE));
    filters.push_back(std::make_pair(
                          converterBSON::VRMatch::New(odil::VR::OW),
                          FilterAction::EXCLUDE));
    filters.push_back(std::make_pair(
                          converterBSON::VRMatch::New(odil::VR::UN),
                          FilterAction::EXCLUDE));

    // Convert Dataset into BSON object
    mongo::BSONObj object = as_bson(dataset, FilterAction::INCLUDE, filters);
    if (!object.isValid() || object.isEmpty())
    {
        return odil::message::CStoreResponse::ErrorCannotUnderstand;
    }

    // Check user's constraints (user's Rights)
    if (!this->is_dataset_allowed_for_storage(username, object))
    {
        return odil::message::CStoreResponse::RefusedNotAuthorized;
    }

    mongo::BSONObjBuilder meta_data_builder;
    meta_data_builder.appendElements(object);

    odil::DataSet meta_information;
    meta_information.add(odil::registry::SourceApplicationEntityTitle,
                         odil::Element({callingaet}, odil::VR::AE));

    std::stringstream stream_dataset;
    odil::Writer::write_file(dataset, stream_dataset, meta_information,
                                odil::registry::ExplicitVRLittleEndian,
                                odil::Writer::ItemEncoding::ExplicitLength);



    // Create a memory buffer with the proper size
    std::string buffer = stream_dataset.str();

    std::string const sopinstanceuid =
            object["00080018"].Obj().getField("Value").Array()[0].String();

    // check size
    auto const use_gridfs = (meta_data_builder.len()+buffer.size()>16777000);

    if (use_gridfs)
    {
        // insert into GridSF
        mongo::GridFS gridfs(this->_database_information.connection, this->get_bulk_data_db());
        mongo::BSONObj gridfs_object =
                gridfs.storeFile(buffer.c_str(),
                                 buffer.size(),
                                 sopinstanceuid);

        if(!gridfs_object.isValid() || gridfs_object.isEmpty())
        {
            // Return an error
            return odil::message::CStoreResponse::ProcessingFailure;
        }

        // Update the meta-data builder with the reference to GridFS
        meta_data_builder << "Content"
                          << gridfs_object.getField("_id").OID().toString();
    }
    else
    {
        if(this->get_db_name() == this->get_bulk_data_db())
        {
            meta_data_builder.appendBinData(
                "Content", buffer.size(), mongo::BinDataGeneral,
                buffer.c_str());
        }
        else
        {
            mongo::BSONObjBuilder bulk_data_builder;
            bulk_data_builder.genOID();
            bulk_data_builder << "SOPInstanceUID" << sopinstanceuid;
            bulk_data_builder.appendBinData(
                "Content", buffer.size(), mongo::BinDataGeneral, buffer.c_str());
            auto bulk_data_object = bulk_data_builder.obj();

            this->_database_information.connection.insert(
                this->get_bulk_data_db()+".datasets", bulk_data_object);

            auto const result = this->_database_information.connection.getLastError(
                this->get_bulk_data_db());
            if(result != "")
            {
                return odil::message::CStoreResponse::ProcessingFailure;
            }

            // Update the meta-data builder with the reference to bulk-data
            meta_data_builder << "Content"
                << bulk_data_object.getField("_id").OID().toString();
        }
    }

    this->_database_information.connection.insert(
            this->get_db_name()+".datasets", meta_data_builder.obj());
    auto const result = this->_database_information.connection.getLastError(
            this->get_db_name());
    if(!result.empty())
    {
        if(use_gridfs)
        {
            mongo::GridFS gridfs(this->_database_information.connection,
                                 this->get_bulk_data_db());
            gridfs.removeFile(sopinstanceuid);
        }
        else if(this->get_db_name() != this->get_bulk_data_db())
        {
            this->_database_information.connection.remove(
                this->get_bulk_data_db()+".datasets",
                BSON("SOPInstanceUID" << sopinstanceuid));
        }
        // Else this->get_db_name() == this->get_bulk_data_db():
        // nothing to roll back

        return odil::message::CStoreResponse::ProcessingFailure;
    }

    return odil::message::CStoreResponse::Success;
}

std::pair<odil::DataSet, odil::DataSet>
MongoDBConnection
::get_dataset(mongo::BSONObj const & object)
{
    if ( ! object.hasField("Content"))
    {
        return std::make_pair(odil::DataSet(), as_dataset(object));
    }

    std::stringstream value;
    value << dataset_as_string(object);

    std::pair<odil::DataSet, odil::DataSet> file;
    try
    {
        file = odil::Reader::read_file(value);
    }
    catch(std::exception & e)
    {
        std::stringstream error;
        error << "Could not read dataset: " << e.what();
        throw ExceptionPACS(error.str());
    }

    return file;
}

std::string MongoDBConnection::as_string(mongo::BSONElement const & bsonelement)
{
    std::stringstream builder;
    switch (bsonelement.type())
    {
        case mongo::BSONType::NumberDouble:
        {
            builder << bsonelement.numberDouble();
            break;
        }
        case mongo::BSONType::String:
        {
            builder << bsonelement.String();
            break;
        }
        case mongo::BSONType::NumberInt:
        {
            builder << bsonelement.Int();
            break;
        }
        case mongo::BSONType::NumberLong:
        {
            builder << bsonelement.Long();
            break;
        }
        default:
        {
            builder << bsonelement.toString(false);
            break;
        }
    }

    return builder.str();
}

std::string MongoDBConnection::dataset_as_string(mongo::BSONObj const & object)
{
    mongo::BSONElement const content = object.getField("Content");
    if (content.type() == mongo::BSONType::String)
    {
        auto const sop_instance_uid =
            object["00080018"].Obj().getField("Value").Array()[0].String();

        mongo::GridFS gridfs(
            this->_database_information.connection, this->get_bulk_data_db());
        auto const file = gridfs.findFile(sop_instance_uid);
        if(file.exists())
        {
            std::stringstream stream;
            file.write(stream);
            return stream.str();
        }
        else
        {
            auto const bulk_data = this->_database_information.connection.findOne(
                this->get_bulk_data_db()+".datasets",
                BSON("SOPInstanceUID" << sop_instance_uid));
            if(bulk_data.isEmpty())
            {
                throw ExceptionPACS("No bulk data");
            }
            auto const bulk_data_content = bulk_data.getField("Content");
            int size=0;
            char const * begin = bulk_data_content.binDataClean(size);
            return std::string(begin, size);
        }
    }
    else if (content.type() == mongo::BSONType::BinData)
    {
        int size=0;
        char const * begin = content.binDataClean(size);

        return std::string(begin, size);
    }
    else
    {
        std::stringstream streamerror;
        streamerror << "Unknown type '" << content.type()
                    << "' for Content attribute";
        throw ExceptionPACS(streamerror.str());
    }

    return "";
}

bool
MongoDBConnection
::is_dataset_allowed_for_storage(std::string const & username,
                                 mongo::BSONObj const & dataset)
{
    // Retrieve user's Rights
    mongo::BSONObj const constraint = this->get_constraints(
                username, odil::message::Message::Command::C_STORE_RQ);

    if (constraint.isEmpty())
    {
        // No constraint
        return true;
    }

    // Compare with input dataset

    // constraint is a Logical OR array
    for (auto itemor : constraint["$or"].Array())
    {
        bool result = true;
        // Each item is a Logical AND array
        for (auto itemand : itemor["$and"].Array())
        {
            // Foreach object
            for(mongo::BSONObj::iterator it=itemand.Obj().begin(); it.more();)
            {
                mongo::BSONElement const element = it.next();

                // Retrieve DICOM field name
                std::string name(element.fieldName());
                name = name.substr(0, 8);

                // Check if input dataset as this field
                if (!dataset.hasField(name))
                {
                    result = false;
                    break;
                }
                else
                {
                    // Compare the field's values
                    auto array = dataset[name].Obj().getField("Value").Array();
                    for(auto itarray = array.begin();
                        itarray != array.end(); ++itarray)
                    {
                        mongo::BSONElement const element2 = *itarray;
                        std::string valuestr = as_string(element2);

                        if (element.type() == mongo::BSONType::RegEx)
                        {
                            std::string regex(element.regex());
                            if (!boost::regex_match(valuestr.c_str(),
                                                    boost::regex(regex.c_str())))
                            {
                                result = false;
                                break;
                            }
                        }
                        else
                        {
                            std::string const elementstr = as_string(element);
                            if (valuestr != elementstr)
                            {
                                result = false;
                                break;
                            }
                        }
                    }
                    if (result == false)
                    {
                        break;
                    }
                }
            }
        }

        // Stop if find dataset match with one condition
        if (result)
        {
            return true;
        }
        // else continue
    }

    return false;
}

std::string
MongoDBConnection
::as_string(odil::message::Message::Command::Type command)
{
    std::string servicename = "";
    switch (command)
    {
    case odil::message::Message::Command::C_ECHO_RQ:
    case odil::message::Message::Command::C_ECHO_RSP:
        servicename = "Echo";
        break;
    case odil::message::Message::Command::C_FIND_RQ:
    case odil::message::Message::Command::C_FIND_RSP:
        servicename = "Query";
        break;
    case odil::message::Message::Command::C_GET_RQ:
    case odil::message::Message::Command::C_GET_RSP:
        servicename = "Retrieve";
        break;
    case odil::message::Message::Command::C_MOVE_RQ:
    case odil::message::Message::Command::C_MOVE_RSP:
        servicename = "Retrieve";
        break;
    case odil::message::Message::Command::C_STORE_RQ:
    case odil::message::Message::Command::C_STORE_RSP:
        servicename = "Store";
        break;
    default:
        break;
    }

    return servicename;
}

std::pair<std::string, int>
MongoDBConnection
::get_peer_information(std::string const & ae_title)
{
    std::string host = "";
    int port = -1;

    mongo::BSONObj const peers_info = this->_database_information.connection.findOne(
        this->get_db_name()+".application_entities",
        BSON("ae_title" << ae_title));

    if (!peers_info.isEmpty())
    {
        host = peers_info.getField("host").String();
        port = (int)peers_info.getField("port").Number();
    }

    return std::make_pair(host, port);
}

} // namespace dopamine
