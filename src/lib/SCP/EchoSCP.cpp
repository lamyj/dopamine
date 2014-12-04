/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "core/LoggerPACS.h"
#include "EchoSCP.h"

namespace research_pacs
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
    research_pacs::loggerInfo() << "Received Echo SCP RQ: MsgID "
                                << this->_request->MessageID;

    return DIMSE_sendEchoResponse(this->_association,
                                  this->_presentationID, 
                                  this->_request, 
                                  STATUS_Success, NULL);
}
    
} // namespace research_pacs
