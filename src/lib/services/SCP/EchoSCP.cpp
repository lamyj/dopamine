/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "core/LoggerPACS.h"
#include "EchoSCP.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace services
{
    
EchoSCP
::EchoSCP(T_ASC_Association * association,
          T_ASC_PresentationContextID presentation_context_id,
          T_DIMSE_C_EchoRQ * request):
    SCP(association, presentation_context_id), // base class initialisation
    _request(request)
{
    // nothing to do
}

EchoSCP
::~EchoSCP()
{
    // nothing to do
}

OFCondition
EchoSCP
::process()
{
    logger_info() << "Received Echo SCP RQ: MsgID "
                 << this->_request->MessageID;

    mongo::DBClientConnection connection;
    std::string db_name;
    bool const connection_state = create_db_connection(connection, db_name);

    // Default response is SUCCESS
    DIC_US status = STATUS_Success;
    DcmDataset * details = NULL;

    if (connection_state)
    {
        std::string const username = get_username(
                    this->_association->params->DULparams.reqUserIdentNeg);

        // Look for user authorization
        if ( ! is_authorized(connection, db_name, username, Service_Echo) )
        {
            // no echo status defined, used STATUS_STORE_Refused_OutOfResources
            status = 0xa700;
            logger_warning() << "User not allowed to perform ECHO";

            create_status_detail(0xa700, DCM_UndefinedTagKey,
                                 OFString("User not allowed to perform ECHO"),
                                 &details);
        }
    }
    else
    {
        // no echo status defined, used STATUS_STORE_Refused_OutOfResources
        status = 0xa700;
        logger_warning() << "Could not connect to database: " << db_name;

        create_status_detail(0xa700, DCM_UndefinedTagKey,
                             OFString("Could not connect to database"),
                             &details);
    }

    // Send the response
    return DIMSE_sendEchoResponse(this->_association,
                                  this->_presentation_context_id,
                                  this->_request,
                                  status, details);
}

} // namespace services
    
} // namespace dopamine
