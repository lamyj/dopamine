/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "QueryGenerator.h"
#include "ServicesTools.h"

namespace dopamine
{

namespace services
{

QueryGenerator
::QueryGenerator(const std::string &username):
    QueryRetrieveGenerator(username, Service_Query) // base class initialisation
{
    // Nothing to do
}

QueryGenerator
::~QueryGenerator()
{
    // Nothing to do
}

} // namespace services

} // namespace dopamine
