/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "Webservices.h"

namespace dopamine
{

namespace services
{

Webservices
::Webservices(std::string const & pathinfo,
              std::string const & querystring):
    _pathinfo(pathinfo), _querystring(querystring),
    _response(""), _boundary(""), _query_retrieve_level("")
{
    // Nothing to do
}

Webservices
::~Webservices()
{
    // Nothing to do
}

std::string
Webservices
::get_response() const
{
    return this->_response;
}

std::string
Webservices
::get_boundary() const
{
    return this->_boundary;
}

void
Webservices
::_create_boundary()
{
    std::stringstream charsstream;
    charsstream << "abcdefghijklmnopqrstuvwxyz"
                << "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                << "1234567890";
    std::string const chars(charsstream.str());
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);

    std::stringstream stream;
    for(int i = 0; i < 15; ++i)
    {
        stream << chars[index_dist(rng)];
    }

    this->_boundary = stream.str();
}

} // namespace services

} // namespace dopamine
