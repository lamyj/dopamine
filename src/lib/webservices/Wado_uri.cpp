/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>

#include <mongo/client/dbclient.h>

#include "core/ConfigurationPACS.h"
#include "core/DBConnection.h" // Warning include DCMTK
#include "Wado_uri.h"
#include "WebServiceException.h"

namespace dopamine
{

namespace webservices
{

std::string wado_uri(std::string const & querystring, std::string &filename)
{
    // Parse the query string
    // WARNING: inadequate method (TODO: find other method)
    // Query string is like: name1=value1&name2=value2
    std::vector<std::string> vartemp;
    boost::split(vartemp, querystring, boost::is_any_of("&"));

    std::map<std::string, std::string> variables;
    for (std::string variable : vartemp)
    {
        std::vector<std::string> data;
        boost::split(data, variable, boost::is_any_of("="));

        if (RequestParameters.find(data[0]) == RequestParameters.end())
        {
            std::stringstream stream;
            stream << "Unknown parameter '" << data[0] << "'";
            throw WebServiceException(400, "Bad Request", stream.str());
        }

        variables.insert(std::pair<std::string,std::string>(data[0], data[1]));
    }
    vartemp.clear();

    // Check Parameters
    for (auto it = RequestParameters.begin();
         it != RequestParameters.end();
         ++it)
    {
        // Look for mandatory information
        if (it->second._mandatory &&
            variables.find(it->first) == variables.end())
        {
            std::stringstream stream;
            stream << "Missing mandatory parameter '" << it->first << "'";
            throw WebServiceException(400, "Bad Request", stream.str());
        }
        // Look for not implemented parameters
        else if (it->second._used == false &&
                 variables.find(it->first) != variables.end())
        {
            std::stringstream stream;
            stream << "Parameter '" << it->first << "' not implemented yet";
            throw WebServiceException(406, "Not Acceptable", stream.str());
        }
    }

    // Look request type
    if (variables[REQUEST_TYPE] != "WADO")
    {
        std::stringstream stream;
        stream << "Unknown value '" << variables[REQUEST_TYPE] << "' for "
               << REQUEST_TYPE << " parameter";
        // No other value allowed for now
        throw WebServiceException(406, "Not Acceptable", stream.str());
    }

    // Request is valid

    // Requested fields
    mongo::BSONObjBuilder fields_builder;
    fields_builder << "00080018" << 1; // SOPInstanceUID
    fields_builder << "0020000d" << 1; // StudyInstanceUID
    fields_builder << "0020000e" << 1; // SeriesInstanceUID
    fields_builder << "location" << 1; // Dataset file path

    // Conditions
    mongo::BSONObjBuilder db_query;
    db_query << "00080018.1" << variables[SOP_INSTANCE_UID]
             << "0020000d.1" << variables[STUDY_INSTANCE_UID]
             << "0020000e.1" << variables[SERIES_INSTANCE_UID];

    // Perform the DB query.
    mongo::BSONObj const fields = fields_builder.obj();

    mongo::BSONObjBuilder initial_builder;
    std::string reduce_function = "function(current, result) { }";
    mongo::BSONObj group_command = BSON("group" << BSON(
        "ns" << "datasets" << "key" << fields << "cond" << db_query.obj() <<
        "$reduce" << reduce_function << "initial" << initial_builder.obj()
    ));

    mongo::BSONObj info;

    // Get all indexes
    std::string indexlist =
        dopamine::ConfigurationPACS::get_instance().GetValue("database.indexlist");
    std::vector<std::string> indexlistvect;
    boost::split(indexlistvect, indexlist, boost::is_any_of(";"));

    // Create and Initialize DB connection
    dopamine::DBConnection connection;

    connection.get_connection().runCommand
        (connection.get_db_name(),
            group_command, info, 0);

    if ( info["count"].Double() == 0)
    {
        throw WebServiceException(404, "Not Found", "Dataset not found");
    }

    std::vector<mongo::BSONElement> results = info["retval"].Array();

    if (results.size() == 0)
    {
        throw WebServiceException(404, "Not Found", "Dataset not found");
    }

    mongo::BSONObj bsonobject = results[0].Obj();
    if (bsonobject.hasField("location") &&
        !bsonobject["location"].isNull() &&
        bsonobject["location"].String() != "")
    {
        std::string value = bsonobject["location"].String();

        filename = boost::filesystem::path(value).filename().c_str();

        // Open file
        std::ifstream dataset(value, std::ifstream::binary | std::ifstream::in);
        if (dataset.is_open())
        {
            // get length of file:
            int length = boost::filesystem::file_size(boost::filesystem::path(value));

            std::string output(length, '\0');

            // read data as a block:
            dataset.read (&output[0], output.size());

            // Close file
            dataset.close();

            return output;
        }
        else
        {
            throw WebServiceException(500, "Internal Server Error", "Unable to open file");
        }
    }
    else
    {
        throw WebServiceException(404, "Not Found", "Dataset is empty");
    }

    return "";
}

} // namespace webservices

} // namespace dopamine
