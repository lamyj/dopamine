/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "EchoSCP.h"

#include <dcmtkpp/message/CEchoResponse.h>
#include <dcmtkpp/message/Response.h>

#include <mongo/client/dbclientinterface.h>

#include "core/LoggerPACS.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace services
{

EchoSCP
::EchoSCP() :
    dcmtkpp::SCP()
{
    // Nothing else.
}

void EchoSCP::operator()(dcmtkpp::message::Message const & message)
{
    mongo::DBClientConnection connection;
    std::string db_name;
    bool const connection_state = create_db_connection(connection, db_name);

    // Default response is SUCCESS
    Uint16 status = dcmtkpp::message::Response::Success;

    if (connection_state)
    {
        std::string const username =
                    this->_association->get_user_identity_primary_field();

        // Look for user authorization
        if ( ! is_authorized(connection, db_name, username, Service_Echo) )
        {
            // no echo status defined, used STATUS_STORE_Refused_OutOfResources
            status = dcmtkpp::message::Response::RefusedNotAuthorized;
            logger_warning() << "User not allowed to perform ECHO";
        }
    }
    else
    {
        // no echo status defined, used STATUS_STORE_Refused_OutOfResources
        status = dcmtkpp::message::Response::RefusedNotAuthorized;
        logger_warning() << "Could not connect to database: " << db_name;
    }

    dcmtkpp::message::CEchoRequest const request(message);

    dcmtkpp::message::CEchoResponse response(
        request.get_message_id(), status,
        request.get_affected_sop_class_uid());

    this->_send(response, request.get_affected_sop_class_uid());
}

} // namespace services

} // namespace dopamine
