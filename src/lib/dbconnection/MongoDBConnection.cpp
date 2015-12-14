/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include <boost/regex.hpp>

#include <dcmtkpp/message/CStoreResponse.h>
#include <dcmtkpp/Exception.h>
#include <dcmtkpp/Reader.h>
#include <dcmtkpp/Tag.h>
#include <dcmtkpp/Writer.h>

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
::MongoDBConnection(std::string const & db_name,
                    std::string const & host_name,
                    int port,
                    std::vector<std::string> const & indexes)
    : _db_name(db_name), _host_name(host_name), _port(port), _indexes(indexes)
{
    // Nothing else.
}

MongoDBConnection
::~MongoDBConnection()
{
    // Nothing to do.
}

mongo::DBClientConnection &
MongoDBConnection
::get_connection()
{
    return this->_connection;
}


std::string
MongoDBConnection
::get_db_name() const
{
    return this->_db_name;
}

void
MongoDBConnection
::set_db_name(std::string const & db_name)
{
    this->_db_name = db_name;
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
    if ( ! this->_connection.connect(mongo::HostAndPort(this->get_host_name(),
                                                        this->get_port()),
                                     errormsg) )
    {
        return false;
    }

    // Create indexes
    std::string const datasets_table = this->get_db_name() + ".datasets";

    for (auto currentIndex : this->get_indexes())
    {
        try
        {
            dcmtkpp::Tag currenttag(currentIndex);

            std::stringstream stream;
            stream << "\"" << std::string(currenttag) << "\"";

            this->_connection.ensureIndex
                (
                    datasets_table,
                    BSON(stream.str() << 1),
                    false,
                    currenttag.get_name()
                );
        }
        catch (dcmtkpp::Exception const & exc)
        {
            logger_warning() << "Ignore index '" << currentIndex
                             << "', reason: " << exc.what();
        }
    }

    return true;
}

bool
MongoDBConnection
::is_authorized(std::string const & user,
                dcmtkpp::message::Message::Command::Type command)
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
    this->_connection.runCommand(this->get_db_name(), group_command, info, 0);

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
                  dcmtkpp::message::Message::Command::Type command)
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
    this->_connection.runCommand(this->get_db_name(), group_command, result, 0);

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
                      int nToSkip, mongo::BSONObj const * fieldsToReturn)
{
    return this->_connection.query(this->get_db_name() + ".datasets", query,
                                   nToReturn, nToSkip, fieldsToReturn);
}

bool MongoDBConnection::run_command(mongo::BSONObj const & command, mongo::BSONObj & response)
{
    bool ret = this->_connection.runCommand(this->get_db_name(),
                                            command, response);

    // If an error occurred
    if (!ret || response["ok"].Double() != 1)
    {
        return false;
    }

    return true;
}

dcmtkpp::Value::Integer
MongoDBConnection
::insert_dataset(std::string const & username,
                 dcmtkpp::DataSet const & dataset,
                 std::string const & callingaet)
{
    Filters filters = {};
    filters.push_back(std::make_pair(
                          converterBSON::IsPrivateTag::New(),
                          FilterAction::EXCLUDE));
    filters.push_back(std::make_pair(
                          converterBSON::VRMatch::New(dcmtkpp::VR::OB),
                          FilterAction::EXCLUDE));
    filters.push_back(std::make_pair(
                          converterBSON::VRMatch::New(dcmtkpp::VR::OF),
                          FilterAction::EXCLUDE));
    filters.push_back(std::make_pair(
                          converterBSON::VRMatch::New(dcmtkpp::VR::OW),
                          FilterAction::EXCLUDE));
    filters.push_back(std::make_pair(
                          converterBSON::VRMatch::New(dcmtkpp::VR::UN),
                          FilterAction::EXCLUDE));

    // Convert Dataset into BSON object
    mongo::BSONObj object = as_bson(dataset, FilterAction::INCLUDE, filters);
    if (!object.isValid() || object.isEmpty())
    {
        return dcmtkpp::message::CStoreResponse::ErrorCannotUnderstand;
    }

    // Check user's constraints (user's Rights)
    if (!this->is_dataset_allowed_for_storage(username, object))
    {
        return dcmtkpp::message::CStoreResponse::RefusedNotAuthorized;
    }

    mongo::BSONObjBuilder builder;
    builder.appendElements(object);

    dcmtkpp::DataSet meta_information;
    meta_information.add(dcmtkpp::registry::SourceApplicationEntityTitle,
                         dcmtkpp::Element({callingaet}, dcmtkpp::VR::AE));

    std::stringstream stream_dataset;
    dcmtkpp::Writer::write_file(dataset, stream_dataset, meta_information,
                                dcmtkpp::registry::ExplicitVRLittleEndian,
                                dcmtkpp::Writer::ItemEncoding::ExplicitLength);



    // Create a memory buffer with the proper size
    std::string buffer = stream_dataset.str();

    std::string const sopinstanceuid =
            object["00080018"].Obj().getField("Value").Array()[0].String();

    // check size
    int const totalsize = builder.len() + buffer.size();

    if (totalsize > 16777000) // 16 MB = 16777216
    {
        // insert into GridSF
        mongo::GridFS gridfs(this->_connection, this->get_db_name());
        mongo::BSONObj objret =
                gridfs.storeFile(buffer.c_str(),
                                 buffer.size(),
                                 sopinstanceuid);

        if (!objret.isValid() || objret.isEmpty())
        {
            // Return an error
            return dcmtkpp::message::CStoreResponse::ProcessingFailure;
        }

        // Prepare for insertion into database
        builder << "Content" << objret.getField("_id").OID().toString();
    }
    else
    {
        // Prepare for insertion into database
        builder.appendBinData("Content", buffer.size(),
                                         mongo::BinDataGeneral, buffer.c_str());
    }

    // insert into db
    std::stringstream stream;
    stream << this->get_db_name() << ".datasets";
    this->_connection.insert(stream.str(), builder.obj());
    std::string result = this->_connection.getLastError(this->get_db_name());
    if (result != "") // empty string if no error
    {
        // Rollback for GridFS
        if (totalsize > 16777000)
        {
            mongo::GridFS gridfs(this->_connection, this->get_db_name());
            gridfs.removeFile(sopinstanceuid);
        }

        // Return an error
        return dcmtkpp::message::CStoreResponse::ProcessingFailure;
    }

    return dcmtkpp::message::CStoreResponse::Success;
}

std::pair<dcmtkpp::DataSet, dcmtkpp::DataSet>
MongoDBConnection
::get_dataset(mongo::BSONObj const & object)
{
    if ( ! object.hasField("Content"))
    {
        return std::make_pair(dcmtkpp::DataSet(), as_dataset(object));
    }

    std::stringstream value;
    value << dataset_as_string(object);

    std::pair<dcmtkpp::DataSet, dcmtkpp::DataSet> file;
    try
    {
        file = dcmtkpp::Reader::read_file(value);
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
    mongo::BSONElement const element = object.getField("Content");
    if (element.type() == mongo::BSONType::String)
    {
        std::string const filesid = element.String();

        // Retrieve Filename
        mongo::BSONObjBuilder builder;
        mongo::OID oid(filesid);
        builder.appendOID(std::string("_id"), &oid);
        mongo::Query const query = builder.obj();
        mongo::BSONObj const fields = BSON("filename" << 1);
        mongo::BSONObj const sopinstanceuidobj =
                this->_connection.findOne(this->get_db_name() + ".fs.files",
                                          query, &fields);
        std::string const sopinstanceuid =
                sopinstanceuidobj.getField("filename").String();

        // Create GridFS interface
        mongo::GridFS gridfs(this->_connection, this->get_db_name());

        // Get the GridFile corresponding to the filename
        mongo::GridFile const file = gridfs.findFile(sopinstanceuid);

        // Get the binary content
        std::stringstream stream;
        auto size = file.write(stream);
        return stream.str();
    }
    else if (element.type() == mongo::BSONType::BinData)
    {
        int size=0;
        char const * begin = element.binDataClean(size);

        return std::string(begin, size);
    }
    else
    {
        std::stringstream streamerror;
        streamerror << "Unknown type '" << element.type()
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
                username, dcmtkpp::message::Message::Command::C_STORE_RQ);

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
::as_string(dcmtkpp::message::Message::Command::Type command)
{
    std::string servicename = "";
    switch (command)
    {
    case dcmtkpp::message::Message::Command::C_ECHO_RQ:
    case dcmtkpp::message::Message::Command::C_ECHO_RSP:
        servicename = "Echo";
        break;
    case dcmtkpp::message::Message::Command::C_FIND_RQ:
    case dcmtkpp::message::Message::Command::C_FIND_RSP:
        servicename = "Query";
        break;
    case dcmtkpp::message::Message::Command::C_GET_RQ:
    case dcmtkpp::message::Message::Command::C_GET_RSP:
        servicename = "Retrieve";
        break;
    case dcmtkpp::message::Message::Command::C_MOVE_RQ:
    case dcmtkpp::message::Message::Command::C_MOVE_RSP:
        servicename = "Retrieve";
        break;
    case dcmtkpp::message::Message::Command::C_STORE_RQ:
    case dcmtkpp::message::Message::Command::C_STORE_RSP:
        servicename = "Store";
        break;
    default:
        break;
    }

    return servicename;
}

} // namespace dopamine