/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/CEchoResponse.h>
#include <dcmtkpp/message/CFindResponse.h>
#include <dcmtkpp/message/CGetResponse.h>
#include <dcmtkpp/message/CMoveResponse.h>
#include <dcmtkpp/message/CStoreResponse.h>

#include "core/Callbacks.h"

namespace dopamine
{

dcmtkpp::Value::Integer echo(dcmtkpp::Association const & association,
                             dcmtkpp::message::CEchoRequest const & request,
                             services::Generator::Pointer generator)
{
    if (generator->done())
    {
        return dcmtkpp::message::CEchoResponse::Success;
    }

    return generator->next();
}

dcmtkpp::Value::Integer find(dcmtkpp::Association const & association,
                             dcmtkpp::message::CFindRequest const & request,
                             services::Generator::Pointer generator)
{
    if (generator->done())
    {
        return dcmtkpp::message::CFindResponse::Success;
    }

    return generator->next();
}

dcmtkpp::Value::Integer get(dcmtkpp::Association const & association,
                            dcmtkpp::message::CGetRequest const & request,
                            services::Generator::Pointer generator)
{
    if (generator->done())
    {
        return dcmtkpp::message::CGetResponse::Success;
    }

    return generator->next();
}

dcmtkpp::Value::Integer move(dcmtkpp::Association const & association,
                             dcmtkpp::message::CMoveRequest const & request,
                             services::Generator::Pointer generator)
{
    if (generator->done())
    {
        return dcmtkpp::message::CMoveResponse::Success;
    }

    return generator->next();
}

dcmtkpp::Value::Integer store(dcmtkpp::Association const & association,
                              dcmtkpp::message::CStoreRequest const & request,
                              services::Generator::Pointer generator)
{
    if (generator->done())
    {
        return dcmtkpp::message::CMoveResponse::Success;
    }

    return generator->next();
}

} // namespace dopamine
