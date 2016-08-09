/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/archive/echo.h"

#include <mongo/client/dbclient.h>

#include <odil/AssociationParameters.h>
#include <odil/message/CEchoRequest.h>
#include <odil/message/Response.h>
#include <odil/Value.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/utils.h"

namespace dopamine
{

namespace archive
{

odil::Value::Integer echo(
    mongo::DBClientConnection const & connection,
    AccessControlList const & acl,
    odil::AssociationParameters const & parameters,
    odil::message::CEchoRequest const & /* not used */)
{
    odil::Value::Integer status;
    if(connection.isFailed())
    {
        status = odil::message::Response::ProcessingFailure;
    }
    else if(!acl.is_allowed(get_principal(parameters), "Echo"))
    {
        status = odil::message::Response::ProcessingFailure;
    }
    else
    {
        status = odil::message::Response::Success;
    }
    return status;
}

} // namespace archive

} // namespace dopamine
