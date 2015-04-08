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

#include "core/ConfigurationPACS.h"
#include "services/RetrieveGenerator.h"
#include "services/ServicesTools.h"
#include "Wado_uri.h"
#include "WebServiceException.h"

namespace dopamine
{

namespace services
{

Wado_uri
::Wado_uri(const std::string &querystring, const std::string &remoteuser):
    Wado(querystring, remoteuser)
{
    mongo::BSONObj object = this->parse_string();

    RetrieveGenerator generator(this->_username);

    Uint16 status = generator.set_query(object);
    if (status != STATUS_Pending)
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

    this->_response = this->get_dataset(findedobject);
}

Wado_uri
::~Wado_uri()
{
    // Nothing to do
}

mongo::BSONObj
Wado_uri
::parse_string()
{
    // Parse the query string
    // WARNING: inadequate method (TODO: find other method)
    // Query string is like: name1=value1&name2=value2
    std::vector<std::string> vartemp;
    boost::split(vartemp, this->_query, boost::is_any_of("&"));

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

    // Conditions
    mongo::BSONObjBuilder db_query;
    db_query << "00080018" << BSON_ARRAY("UI" << variables[SOP_INSTANCE_UID])
             << "0020000d" << BSON_ARRAY("UI" << variables[STUDY_INSTANCE_UID])
             << "0020000e" << BSON_ARRAY("UI" << variables[SERIES_INSTANCE_UID]);

    return db_query.obj();
}

} // namespace services

} // namespace dopamine
