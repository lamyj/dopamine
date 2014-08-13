/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "GetSCP.h"

namespace research_pacs
{

static void getCallback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_GetRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_GetRSP *response, DcmDataset **stDetail,
        DcmDataset **responseIdentifiers)
{
    /*static FindResponseGenerator * generator = NULL;

    OFCondition dbcond = EC_Normal;

    GetSCP * scp = reinterpret_cast<GetCallbackData*>(callbackData)->scp;
    std::string const & ae_title = reinterpret_cast<GetCallbackData*>(callbackData)->ae_title;

    if (responseCount == 1)
    {
        /* start the database search */
    /*    DCMQRDB_INFO("Get SCP Request Identifiers:" << OFendl << DcmObject::PrintHelper(*requestIdentifiers));

        if(generator != NULL)
        {
            delete generator;
        }
        // Include the location field
        //generator = new FindResponseGenerator(
        //    *requestIdentifiers, scp->get_connection(), scp->get_db_name(), true);
    }*/
}
    
GetSCP
::GetSCP(T_ASC_Association * assoc, 
         T_ASC_PresentationContextID presID, 
         T_DIMSE_C_GetRQ * req):
    SCP(assoc, presID), _request(req)
{
    // nothing to do
}

GetSCP
::~GetSCP()
{
    // nothing to do
}

OFCondition 
GetSCP
::process()
{
    std::cout << "Received Get SCP: MsgID " 
              << this->_request->MessageID << std::endl;
              
    GetCallbackData data;
    data.scp = this;

    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(this->_association->params, NULL, aeTitle, NULL);
    data.ae_title = aeTitle;
    
    return DIMSE_getProvider(this->_association, this->_presentationID, 
                             this->_request, getCallback, &data, 
                             DIMSE_BLOCKING, 0);
}

} // namespace research_pacs
