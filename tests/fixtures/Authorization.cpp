/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "fixtures/Authorization.h"

#include "dopamine/AccessControlList.h"

#include "fixtures/MongoDB.h"

namespace fixtures
{

Authorization
::Authorization()
: MongoDB(), acl(this->connection, this->database)
{
    this->acl.set_entries({
        { "echo", "Echo", {} },
        { "store", "Store", {} },
        { "query", "Query", {} },
        { "retrieve", "Retrieve", {} },
        { "all", "*", {} },
    });
}

Authorization
::~Authorization()
{
    // Nothing to do.
}

} // namespace fixtures
