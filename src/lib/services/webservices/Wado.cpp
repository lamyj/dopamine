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
::Wado(std::string const & pathinfo, std::string const & querystring,
       std::string const & username):
    Webservices(pathinfo, querystring, username)
{
    // Nothing to do
}

Wado
::~Wado()
{
    // Nothing to do
}

} // namespace services

} // namespace dopamine
