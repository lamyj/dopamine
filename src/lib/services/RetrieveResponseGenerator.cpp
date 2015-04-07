/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "RetrieveResponseGenerator.h"

namespace dopamine
{

namespace services
{

RetrieveResponseGenerator
::RetrieveResponseGenerator(std::string const & username):
    QueryRetrieveGenerator(username, Service_Retrieve) // base class initialization
{
}

RetrieveResponseGenerator
::~RetrieveResponseGenerator()
{
    // Nothing to do
}

} // namespace services

} // namespace dopamine
