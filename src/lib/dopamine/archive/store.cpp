/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/archive/store.h"

#include <odil/AssociationParameters.h>
#include <odil/message/CStoreRequest.h>
#include <odil/message/CStoreResponse.h>
#include <odil/Value.h>

#include "dopamine/AccessControlList.h"
#include "dopamine/archive/Storage.h"
#include "dopamine/Exception.h"
#include "dopamine/utils.h"

namespace dopamine
{

namespace archive
{

odil::Value::Integer store(
    AccessControlList const & acl,
    odil::AssociationParameters const & parameters,
    dopamine::archive::Storage & storage,
    odil::message::CStoreRequest const & request)
{
    odil::Value::Integer status = odil::message::CStoreResponse::Success;

    if(!acl.is_allowed(get_principal(parameters), "Store"))
    {
        status = odil::message::CStoreResponse::RefusedNotAuthorized;
    }
    else
    {
        auto const & data_set = request.get_data_set();
        try
        {
            storage.store(data_set);
        }
        catch(Exception const & e)
        {
            status = odil::message::CStoreResponse::ProcessingFailure;
        }
    }

    return status;
}

} // namespace archive

} // namespace dopamine
