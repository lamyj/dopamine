/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#include <dcmtk/config/osconfig.h>    /* make sure OS specific configuration is included first */
#include <dcmtk/dcmqrdb/dcmqropt.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmnet/diutil.h>

#include "FindResponseGenerator.h"
#include "FindSCP.h"

namespace research_pacs
{
    
static void findCallback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_FindRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_FindRSP *response,
        DcmDataset **responseIdentifiers,
        DcmDataset **stDetail)
{
    static FindResponseGenerator * generator = NULL;

    OFCondition dbcond = EC_Normal;

    FindSCP * scp = reinterpret_cast<FindCallbackData*>(callbackData)->scp;
    std::string const & ae_title = reinterpret_cast<FindCallbackData*>(callbackData)->ae_title;

    if (responseCount == 1)
    {
        /* start the database search */
        if(generator != NULL)
        {
            delete generator;
        }
        generator = new FindResponseGenerator(*requestIdentifiers);
    }

    /* only cancel if we have pending responses */
    if (cancelled && DICOM_PENDING_STATUS(generator->status()))
    {
        generator->cancel();
    }

    if (DICOM_PENDING_STATUS(generator->status())) {
        dbcond = generator->next(responseIdentifiers);
        if (dbcond.bad())
        {
             DCMQRDB_ERROR("findSCP: Database: nextFindResponse Failed ("
                     << DU_cfindStatusString(generator->status()) << "):");
        }
    }

    if (*responseIdentifiers != NULL)
    {

        if (! DU_putStringDOElement(*responseIdentifiers, DCM_RetrieveAETitle, ae_title.c_str()))
        {
            DCMQRDB_ERROR("DO: adding Retrieve AE Title");
        }
    }

    /* set response status */
    response->DimseStatus = generator->status();
    *stDetail = NULL; // TODO

    OFString str;
    DCMQRDB_INFO("Find SCP Response " << responseCount << " [status: "
            << DU_cfindStatusString(generator->status()) << "]");
}
    
FindSCP
::FindSCP(T_ASC_Association * assoc, 
          T_ASC_PresentationContextID presID, 
          T_DIMSE_C_FindRQ * req):
    SCP(assoc, presID), _request(req)
{
    // nothing to do
}

FindSCP
::~FindSCP()
{
    // nothing to do
}

OFCondition 
FindSCP
::process()
{
    std::cout << "Received Find SCP: MsgID " 
              << this->_request->MessageID << std::endl;
              
    FindCallbackData data;
    data.scp = this;

    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(this->_association->params, NULL, aeTitle, NULL);
    data.ae_title = aeTitle;
    
    return DIMSE_findProvider(this->_association, this->_presentationID, 
                              this->_request, findCallback, &data, 
                              DIMSE_BLOCKING, 0);
}

} // namespace research_pacs
