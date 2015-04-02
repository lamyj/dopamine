/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "ConverterBSON/BSONToDataSet.h"
#include "ConverterBSON/DataSetToBSON.h"
#include "ConverterBSON/TagMatch.h"
#include "core/LoggerPACS.h"
#include "FindResponseGenerator.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace services
{

FindResponseGenerator
::FindResponseGenerator(T_ASC_Association *request_association):
    ResponseGenerator(request_association, Service_Query) // base class initialisation
{
    // Nothing to do
}

FindResponseGenerator
::~FindResponseGenerator()
{
    // Nothing to do
}

void 
FindResponseGenerator
::process(
    /* in */
    OFBool cancelled, T_DIMSE_C_FindRQ* request,
    DcmDataset* requestIdentifiers, int responseCount,
    /* out */
    T_DIMSE_C_FindRSP* response, DcmDataset** responseIdentifiers,
    DcmDataset** stDetail)
{
    if (responseCount == 1)
    {
        this->_status = this->set_query(requestIdentifiers);

        if (this->_status != STATUS_Success && this->_status != STATUS_Pending)
        {
            response->DimseStatus = this->_status;
            createStatusDetail(this->_status, DCM_UndefinedTagKey,
                               OFString("An error occured while processing Find operation"),
                               stDetail);
        }
    } // if (responseCount == 1)
    
    /* only cancel if we have pending responses */
    if (cancelled && this->_status == STATUS_Pending)
    {
        this->cancel();
    }
    
    /* Process next result */
    if (this->_status == STATUS_Pending)
    {
        this->next(responseIdentifiers, stDetail);
    }
    
    /* set response status */
    response->DimseStatus = this->_status;
    if (this->_status == STATUS_Pending || this->_status == STATUS_Success)
    {
        *stDetail = NULL;
    }
}

void
FindResponseGenerator
::next(DcmDataset ** responseIdentifiers, DcmDataset ** details)
{
    *details = NULL;
    if( ! this->_cursor->more() )
    {
        // We're done.
        this->_status = STATUS_Success;
    }
    else
    {
        BSONToDataSet bson_to_dataset;
        mongo::BSONObj item = this->_cursor->next();

        {
            DcmDataset dataset = bson_to_dataset(item);
            (*responseIdentifiers) = new DcmDataset(dataset);
        }
        
        OFCondition condition =
                (*responseIdentifiers)->putAndInsertOFStringArray(DCM_QueryRetrieveLevel,
                                          this->_query_retrieve_level.c_str());

        if (condition.bad())
        {
            dopamine::loggerError() << "Cannot insert DCM_QueryRetrieveLevel: "
                                    << condition .text();

            this->_status = STATUS_FIND_Failed_UnableToProcess;

            createStatusDetail(STATUS_FIND_Failed_UnableToProcess,
                               DCM_QueryRetrieveLevel, OFString(condition.text()), details);
            return;
        }

        if(item.hasField("instance_count"))
        {
            OFString count(12, '\0');
            snprintf(&count[0], 12, "%i", int(item.getField("instance_count").Number()));
            condition = (*responseIdentifiers)->putAndInsertOFStringArray(this->_instance_count_tag,
                                              count);

            if (condition.bad())
            {
                dopamine::loggerError() << "Cannot insert "
                                        << this->_instance_count_tag.getGroup() << ","
                                        << this->_instance_count_tag.getElement() << ": "
                                        << condition .text();

                this->_status = STATUS_FIND_Failed_UnableToProcess;

                createStatusDetail(STATUS_FIND_Failed_UnableToProcess,
                                   this->_instance_count_tag,
                                   OFString(condition.text()), details);
                return;
            }
        }
        if(this->_convert_modalities_in_study)
        {
            (*responseIdentifiers)->remove(DCM_Modality);
            std::vector<mongo::BSONElement> const modalities = 
                item.getField("modalities_in_study").Array();
            std::string value;
            for(unsigned int i=0; i<modalities.size(); ++i)
            {
                value += modalities[i].String();
                if(i!=modalities.size()-1)
                {
                    value += "\\";
                }
            }
            condition = (*responseIdentifiers)->putAndInsertOFStringArray(DCM_ModalitiesInStudy,
                                                              OFString(value.c_str()));

            if (condition.bad())
            {
                dopamine::loggerError() << "Cannot insert DCM_ModalitiesInStudy: "
                                        << condition .text();

                this->_status = STATUS_FIND_Failed_UnableToProcess;

                createStatusDetail(STATUS_FIND_Failed_UnableToProcess,
                                   DCM_ModalitiesInStudy,
                                   OFString(condition.text()), details);
                return;
            }
        }

        this->_status = STATUS_Pending;
    }
}

} // namespace services

} // namespace dopamine
