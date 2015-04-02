/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "core/LoggerPACS.h"
#include "MoveSCP.h"
#include "services/MoveResponseGenerator.h"

namespace dopamine
{

namespace services
{

/**
 * Callback handler called by the DIMSE_moveProvider callback function
 * @param callbackdata: Callback context (in)
 * @param cancelled: flag indicating wheter a C-CANCEL was received (in)
 * @param request: original move request (in)
 * @param requestIdentifiers: original move request identifiers (in)
 * @param responseCount: move response count (in)
 * @param response: final move response (out)
 * @param stDetail: status detail for move response (out)
 * @param responseIdentifiers: move response identifiers (out)
 */
static void moveCallback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_MoveRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_MoveRSP* response,
        DcmDataset** stDetail, DcmDataset** responseIdentifiers)
{
    MoveResponseGenerator* context =
            reinterpret_cast<MoveResponseGenerator*>(callbackData);
    context->process(cancelled, request, requestIdentifiers,
                     responseCount, response, stDetail,
                     responseIdentifiers);
}
    
MoveSCP
::MoveSCP(T_ASC_Association * assoc, 
          T_ASC_PresentationContextID presID,
          T_DIMSE_C_MoveRQ * req):
    services::SCP(assoc, presID), _request(req) // base class initialisation
{
    // Nothing to do
}

MoveSCP
::~MoveSCP()
{
    // Nothing to do
}

OFCondition
MoveSCP
::process()
{
   dopamine::loggerInfo() << "Received Move SCP: MsgID "
                               << this->_request->MessageID;

    MoveResponseGenerator context(this->_association);
    
    return DIMSE_moveProvider(this->_association, this->_presentationID, 
                              this->_request, moveCallback, &context, 
                              DIMSE_BLOCKING, 0);
}

void
MoveSCP
::set_network(T_ASC_Network *network)
{
    this->_network = network;
}

} // namespace services
    
} // namespace dopamine
