/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "RetrieveGenerator.h"
#include "ServicesTools.h"

namespace dopamine
{

namespace services
{

RetrieveGenerator
::RetrieveGenerator(std::string const & username):
    QueryRetrieveGenerator(username, Service_Retrieve)
{
}

RetrieveGenerator
::~RetrieveGenerator()
{
    // Nothing to do
}

} // namespace services

} // namespace dopamine