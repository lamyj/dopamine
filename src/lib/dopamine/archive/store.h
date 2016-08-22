/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _92e06111_0a0f_43ee_9cd4_4f1a5458bf24
#define _92e06111_0a0f_43ee_9cd4_4f1a5458bf24

#include <odil/AssociationParameters.h>
#include <odil/message/CStoreRequest.h>
#include <odil/Value.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/archive/Storage.h"

namespace dopamine
{

namespace archive
{

/// @brief Store callback.
odil::Value::Integer store(
    AccessControlList const & acl,
    odil::AssociationParameters const & parameters,
    dopamine::archive::Storage & storage,
    odil::message::CStoreRequest const & request);

} // namespace archive

} // namespace dopamine


#endif // _92e06111_0a0f_43ee_9cd4_4f1a5458bf24
