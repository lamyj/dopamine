/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _2820e16f_8d19_4094_89f9_3bc64631fd72
#define _2820e16f_8d19_4094_89f9_3bc64631fd72

#include <mongo/client/dbclient.h>

#include <odil/AssociationParameters.h>
#include <odil/message/CEchoRequest.h>
#include <odil/Value.h>

#include "dopamine/AccessControlList.h"

namespace dopamine
{

namespace archive
{

/// @brief Echo callback checking that the DB connection is alive.
odil::Value::Integer echo(
    mongo::DBClientConnection const & connection,
    AccessControlList const & acl,
    odil::AssociationParameters const & parameters,
    odil::message::CEchoRequest const & request);

} // namespace archive

} // namespace dopamine

#endif // _2820e16f_8d19_4094_89f9_3bc64631fd72
