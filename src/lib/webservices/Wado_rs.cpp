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
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "core/ConfigurationPACS.h"
#include "core/DBConnection.h"
#include "Wado_rs.h"
#include "WebServiceException.h"

namespace dopamine
{

namespace webservices
{

Wado_rs::Wado_rs(const std::string &pathinfo):
    _filename(""), _study_instance_uid(""), _series_instance_uid(""),
    _sop_instance_uid(""), _response(""), _boundary("")
{
    _results.clear();

    this->parse_pathfinfo(pathinfo);

    this->search_database();

    this->create_response();
}

Wado_rs::~Wado_rs()
{
    // release connexion initialized in function search_database
    dopamine::DBConnection::delete_instance();
}

void Wado_rs::parse_pathfinfo(const std::string &pathinfo)
{
    // Parse the path info
    // WARNING: inadequate method (TODO: find other method)
    // PATH_INFO is like: key1/value1/key2/value2
    std::vector<std::string> vartemp;
    boost::split(vartemp, pathinfo, boost::is_any_of("/"),
                 boost::token_compress_off);

    if (vartemp.size() > 0 && vartemp[0] == "")
    {
        vartemp.erase(vartemp.begin());
    }

    if (vartemp.size() < 1)
    {
        throw WebServiceException(400, "Bad Request",
                                  "Some parameters missing");
    }

    // look for Study Instance UID
    if (vartemp[0] == "studies")
    {
        if (vartemp.size() < 2 || vartemp[1] == "")
        {
            throw WebServiceException(400, "Bad Request",
                                      "Missing study instance uid");
        }

        this->_study_instance_uid = vartemp[1];

        // look for Series Instance UID
        if (vartemp.size() > 2)
        {
            if (vartemp[2] != "series")
            {
                throw WebServiceException(400, "Bad Request",
                                          "second parameter should be series");
            }

            if (vartemp.size() < 4 || vartemp[3] == "")
            {
                throw WebServiceException(400, "Bad Request",
                                          "Missing series instance uid");
            }

            this->_series_instance_uid = vartemp[3];

            // look for SOP Instance UID
            if (vartemp.size() > 4)
            {
                if (vartemp[4] != "instances")
                {
                    throw WebServiceException(400, "Bad Request",
                                              "third parameter should be instances");
                }

                if (vartemp.size() < 6 || vartemp[5] == "")
                {
                    throw WebServiceException(400, "Bad Request",
                                              "Missing SOP instance uid");
                }

                this->_sop_instance_uid = vartemp[5];
            }
        }
    }
    else
    {
        throw WebServiceException(400, "Bad Request",
                                  "first parameter should be studies");
    }

    // Request is valid
}

void Wado_rs::search_database()
{
    // Requested fields
    mongo::BSONObjBuilder fields_builder;

    // Conditions
    mongo::BSONObjBuilder db_query;

    if (this->_sop_instance_uid != "")
    {
        fields_builder << "00080018" << 1; // SOPInstanceUID
        db_query << "00080018.1" << this->_sop_instance_uid;
    }

    if (this->_study_instance_uid != "")
    {
        fields_builder << "0020000d" << 1; // StudyInstanceUID
        db_query << "0020000d.1" << this->_study_instance_uid;
    }

    if (this->_series_instance_uid != "")
    {
        fields_builder << "0020000e" << 1; // SeriesInstanceUID
        db_query << "0020000e.1" << this->_series_instance_uid;
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

    this->_results = info["retval"].Array();

    if (this->_results.size() == 0)
    {
        throw WebServiceException(404, "Not Found", "Dataset not found");
    }
}

void Wado_rs::create_response()
{
    // Multipart Response
    this->create_boundary();

    std::stringstream stream;
    for (unsigned int i = 0; i < this->_results.size(); ++i)
    {
        mongo::BSONObj bsonobject = this->_results[i].Obj();

        if (bsonobject.hasField("location") &&
            !bsonobject["location"].isNull() &&
            bsonobject["location"].String() != "")
        {
            std::string value = bsonobject["location"].String();

            this->_filename = boost::filesystem::path(value).filename().c_str();

            stream << "--" << this->_boundary << "\n";
            stream << CONTENT_TYPE << MIME_TYPE_APPLICATION_DICOM << "\n";
            stream << CONTENT_DISPOSITION_ATTACHMENT << " "
                   << ATTRIBUT_FILENAME << this->_filename << "\n";
            stream << CONTENT_TRANSFER_ENCODING << TRANSFER_ENCODING_BINARY << "\n" << "\n";

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

                stream << output << "\n" << "\n";
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
    }

    // Close the response
    stream << "--" << this->_boundary << "--";

    this->_response = stream.str();
}

void Wado_rs::create_boundary()
{
    std::string const chars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);

    std::stringstream stream;
    for(int i = 0; i < 15; ++i)
    {
        stream << chars[index_dist(rng)];
    }

    this->_boundary = stream.str();
}

} // namespace webservices

} // namespace dopamine
