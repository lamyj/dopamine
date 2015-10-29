/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include <dcmtkpp/Response.h>

#include "services/RetrieveGenerator.h"
#include "Wado_uri.h"
#include "WebServiceException.h"

namespace dopamine
{

namespace services
{

Wado_uri
::Wado_uri(std::string const & querystring,
           std::string const & remoteuser):
    Wado("", querystring, remoteuser), _filename("")
{
    mongo::BSONObj const object = this->_parse_string();

    RetrieveGenerator generator(this->_username);

    Uint16 const status = generator.process_bson(object);
    if (status != dcmtkpp::Response::Pending)
    {
        if ( ! generator.is_allow())
        {
            throw WebServiceException(401, "Authorization Required",
                                      authentication_string);
        }

        throw WebServiceException(500, "Internal Server Error",
                                  "Error while searching into database");
    }

    mongo::BSONObj findedobject = generator.next();
    if (!findedobject.isValid() || findedobject.isEmpty())
    {
        throw WebServiceException(404, "Not Found", "No Dataset");
    }

    try
    {
        this->_response = generator.retrieve_dataset_as_string(findedobject);
    }
    catch (ExceptionPACS const & exc)
    {
        throw WebServiceException(500, "Internal Server Error", exc.what());
    }
}

Wado_uri
::~Wado_uri()
{
    // Nothing to do
}

std::string
Wado_uri
::get_filename() const
{
    return this->_filename;
}

mongo::BSONObj
Wado_uri
::_parse_string()
{
    // Parse the query string
    // WARNING: inadequate method (TODO: find other method)
    // Query string is like: name1=value1&name2=value2
    std::vector<std::string> vartemp;
    boost::split(vartemp, this->_querystring, boost::is_any_of("&"));

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
        if (it->second.is_mandatory() &&
            variables.find(it->first) == variables.end())
        {
            std::stringstream stream;
            stream << "Missing mandatory parameter '" << it->first << "'";
            throw WebServiceException(400, "Bad Request", stream.str());
        }
        // Look for not implemented parameters
        else if (it->second.is_used() == false &&
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

    this->_filename = variables[SOP_INSTANCE_UID];

    // Conditions
    mongo::BSONObjBuilder db_query;
    db_query << "00080018"
             << BSON("vr" << "UI" <<
                     "Value" << BSON_ARRAY(variables[SOP_INSTANCE_UID]))
             << "0020000d"
             << BSON("vr" << "UI" <<
                     "Value" << BSON_ARRAY(variables[STUDY_INSTANCE_UID]))
             << "0020000e"
             << BSON("vr" << "UI" <<
                     "Value" << BSON_ARRAY(variables[SERIES_INSTANCE_UID]));

    db_query << "00080052"
             << BSON("vr" << "CS" << "Value" << BSON_ARRAY("IMAGE"));

    return db_query.obj();
}

} // namespace services

} // namespace dopamine
