/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>

#include "core/ConfigurationPACS.h"
#include "core/DBConnection.h"
#include "Wado_rs.h"
#include "WebServiceException.h"

namespace dopamine
{

namespace webservices
{

std::string wado_rs(std::string const & pathinfo, std::string &filename)
{
    // Parse the path info
    // WARNING: inadequate method (TODO: find other method)
    // PATH_INFO is like: key1/value1/key2/value2
    std::vector<std::string> vartemp;
    boost::split(vartemp, pathinfo, boost::is_any_of("/"), boost::token_compress_off);

    if (vartemp.size() > 0 && vartemp[0] == "")
    {
        vartemp.erase(vartemp.begin());
    }

    if (vartemp.size() < 1)
    {
        throw WebServiceException(400, "Bad Request", "Some parameters missing");
    }

    std::string study_instance_uid = "";
    std::string series_instance_uid = "";
    std::string sop_instance_uid = "";

    // look for Study Instance UID
    if (vartemp[0] == "studies")
    {
        if (vartemp.size() < 2)
        {
            throw WebServiceException(400, "Bad Request", "Missing study instance uid");
        }

        study_instance_uid = vartemp[1];

        // look for Series Instance UID
        if (vartemp.size() > 2)
        {
            if (vartemp[2] != "series")
            {
                throw WebServiceException(400, "Bad Request", "second parameter should be series");
            }

            if (vartemp.size() < 4)
            {
                throw WebServiceException(400, "Bad Request", "Missing series instance uid");
            }

            series_instance_uid = vartemp[3];

            // look for SOP Instance UID
            if (vartemp.size() > 4)
            {
                if (vartemp[4] != "instances")
                {
                    throw WebServiceException(400, "Bad Request", "third parameter should be instances");
                }

                if (vartemp.size() < 6)
                {
                    throw WebServiceException(400, "Bad Request", "Missing SOP instance uid");
                }

                sop_instance_uid = vartemp[5];
            }
        }
    }
    else
    {
        throw WebServiceException(400, "Bad Request", "first parameter should be studies");
    }
    vartemp.clear();

    // Request is valid

    // Requested fields
    mongo::BSONObjBuilder fields_builder;

    // Conditions
    mongo::BSONObjBuilder db_query;

    if (sop_instance_uid != "")
    {
        fields_builder << "00080018" << 1; // SOPInstanceUID
        db_query << "00080018.1" << sop_instance_uid;
    }

    if (study_instance_uid != "")
    {
        fields_builder << "0020000d" << 1; // StudyInstanceUID
        db_query << "0020000d.1" << study_instance_uid;
    }

    if (series_instance_uid != "")
    {
        fields_builder << "0020000e" << 1; // SeriesInstanceUID
        db_query << "0020000e.1" << series_instance_uid;
    }

    fields_builder << "location" << 1; // Dataset file path

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
    dopamine::DBConnection::get_instance().Initialize
        (
            dopamine::ConfigurationPACS::get_instance().GetValue("database.dbname"),
            dopamine::ConfigurationPACS::get_instance().GetValue("database.hostname"),
            dopamine::ConfigurationPACS::get_instance().GetValue("database.port"),
            indexlistvect
        );

    // Connect Database
    dopamine::DBConnection::get_instance().connect();

    DBConnection::get_instance().get_connection().runCommand
        (DBConnection::get_instance().get_db_name(),
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
    if(bsonobject.hasField("location") && !bsonobject["location"].isNull())
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

    return pathinfo;
}

} // namespace webservices

} // namespace dopamine
