/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "core/LoggerPACS.h"
#include "core/NetworkPACS.h"
#include "EchoSCP.h"
#include "ResponseGenerator.h"

namespace dopamine
{
    
EchoSCP
::EchoSCP(T_ASC_Association * assoc, 
          T_ASC_PresentationContextID presID, 
          T_DIMSE_C_EchoRQ * req):
    SCP(assoc, presID), _request(req) // base class initialisation
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
    loggerInfo() << "Received Echo SCP RQ: MsgID "
                 << this->_request->MessageID;

    // Default response is SUCCESS
    DIC_US status = STATUS_Success;
    DcmDataset * details = NULL;

    // Look for user authorization
    if ( !NetworkPACS::get_instance().check_authorization(this->_association->params->DULparams.reqUserIdentNeg,
                                                          Service_Echo) )
    {
        status = 0xa700; // no echo status defined, used STATUS_STORE_Refused_OutOfResources
        loggerWarning() << "User not allowed to perform ECHO";

        ResponseGenerator::createStatusDetail(0xa700, DCM_UndefinedTagKey,
                                              OFString("User not allowed to perform ECHO"),
                                              &details);
    }

    // Send the response
    return DIMSE_sendEchoResponse(this->_association,
                                  this->_presentationID, 
                                  this->_request, 
                                  status, details);
}
    
} // namespace dopamine
