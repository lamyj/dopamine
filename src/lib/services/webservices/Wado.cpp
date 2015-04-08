/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>

#include "Wado.h"
#include "WebServiceException.h"

namespace dopamine
{

namespace services
{

Wado
::Wado(const std::string &query, std::string const & username):
    _query(query), _username(username),
    _filename(""), _response("")
{
    // Nothing to do
}

Wado
::~Wado()
{
    // Nothing to do
}

std::string
Wado
::get_filename() const
{
    return this->_filename;
}

std::string
Wado
::get_response() const
{
    return this->_response;
}

std::string Wado::get_dataset(const mongo::BSONObj &object)
{
    std::stringstream stream;
    if (object.hasField("location") &&
        !object["location"].isNull() &&
        object["location"].String() != "")
    {
        std::string value = object["location"].String();

        this->_filename = boost::filesystem::path(value).filename().c_str();

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

            stream << output;
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

    return stream.str();
}

} // namespace services

} // namespace dopamine
