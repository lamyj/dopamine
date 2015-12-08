/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <string>

#include <dcmtkpp/Exception.h>
#include <dcmtkpp/message/CEchoResponse.h>
#include <dcmtkpp/message/CFindResponse.h>
#include <dcmtkpp/message/CGetResponse.h>
#include <dcmtkpp/message/CMoveResponse.h>
#include <dcmtkpp/message/CStoreResponse.h>

#include <mongo/client/dbclientinterface.h>

#include "core/Callbacks.h"
#include "core/LoggerPACS.h"
#include "services/ServicesTools.h"
//#include "services/StoreGenerator.h"

namespace dopamine
{

dcmtkpp::Value::Integer echo(dcmtkpp::Association const & association,
                             dcmtkpp::message::CEchoRequest const & request,
                             services::Generator::Pointer generator)
{
    mongo::DBClientConnection connection;
    std::string db_name;
    bool const connection_state = services::create_db_connection(connection, db_name);

    if (connection_state)
    {
        std::string const username =
                    association.get_user_identity_primary_field();

        // Look for user authorization
        if ( ! services::is_authorized(connection, db_name, username, services::Service_Echo) )
        {
            logger_warning() << "User not allowed to perform ECHO";
            throw dcmtkpp::Exception("User not allowed to perform ECHO");
        }
    }
    else
    {
        std::stringstream error;
        error << "Could not connect to database: " << db_name;
        logger_warning() << error.str();
        throw dcmtkpp::Exception(error.str());
    }

    return dcmtkpp::message::CEchoResponse::Success;
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
