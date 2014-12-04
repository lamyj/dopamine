/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "ConverterBSON/BSONToDataSet.h"
#include "ConverterBSON/DataSetToBSON.h"
#include "ConverterBSON/TagMatch.h"
#include "core/DBConnection.h"
#include "core/ExceptionPACS.h"
#include "core/LoggerPACS.h"
#include "GetResponseGenerator.h"

namespace research_pacs
{
    
/**
 * Callback handler called by the DIMSE_storeProvider callback function
 * @param progress: progress state (in)
 * @param request: original store request (in)
 */
static void getSubProcessCallback(void*, T_DIMSE_StoreProgress * progress,
                                  T_DIMSE_C_StoreRQ* request)
{
    // Nothing to do
}
    
GetResponseGenerator
::GetResponseGenerator(GetSCP * scp, std::string const & ouraetitle):
    ResponseGenerator(scp, ouraetitle) // base class initialisation
{
    // Nothing to do
}

GetResponseGenerator
::~GetResponseGenerator()
{
    // Nothing to do
}
    
void 
GetResponseGenerator
::callBackHandler(
    /* in */
    OFBool cancelled, T_DIMSE_C_GetRQ* request,
    DcmDataset* requestIdentifiers, int responseCount,
    /* out */
    T_DIMSE_C_GetRSP* response, DcmDataset** stDetail,
    DcmDataset** responseIdentifiers)
{
    if (responseCount == 1)
    {
        /* Start the database search */
        
        // Convert the dataset to BSON, excluding Query/Retrieve Level.
        DataSetToBSON dataset_to_bson;

        dataset_to_bson.get_filters().push_back(
            std::make_pair(TagMatch::New(DCM_QueryRetrieveLevel),
                           DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(
            std::make_pair(TagMatch::New(DCM_SpecificCharacterSet),
                           DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.set_default_filter(DataSetToBSON::FilterAction::INCLUDE);

        mongo::BSONObjBuilder query_builder;
        dataset_to_bson(requestIdentifiers, query_builder);
        mongo::BSONObj const query_dataset = query_builder.obj();

        // Build the MongoDB query and query fields from the query dataset.
        mongo::BSONObjBuilder db_query;
        mongo::BSONObjBuilder fields_builder;
        for(mongo::BSONObj::iterator it=query_dataset.begin(); it.more();)
        {
            mongo::BSONElement const element = it.next();
            std::vector<mongo::BSONElement> const array = element.Array();

            std::string const vr = array[0].String();
            mongo::BSONElement const & value = array[1];
            Match::Type const match_type = this->_get_match_type(vr, value);

            DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);
            // Match the array element containing the value
            (this->*function)(std::string(element.fieldName())+".1", vr, value, db_query);
        }
        
        // retrieve 'location' field
        fields_builder << "location" << 1;

        // Always include Specific Character Set in results.
        if(!fields_builder.hasField("00080005"))
        {
            fields_builder << "00080005" << 1;
        }

        // Always include the keys for the query level and its higher levels
        OFString ofstring;
        requestIdentifiers->findAndGetOFString(DCM_QueryRetrieveLevel, ofstring);
        this->_query_retrieve_level = std::string(ofstring.c_str());
        if(!fields_builder.hasField("00100020"))
        {
            fields_builder << "00100020" << 1;
        }
        if((this->_query_retrieve_level=="STUDY" ||
                 this->_query_retrieve_level=="SERIES" ||
                 this->_query_retrieve_level=="IMAGE") && !fields_builder.hasField("0020000d"))
        {
            fields_builder << "0020000d" << 1;
        }
        if((this->_query_retrieve_level=="SERIES" ||
                 this->_query_retrieve_level=="IMAGE") && !fields_builder.hasField("0020000e"))
        {
            fields_builder << "0020000e" << 1;
        }
        if(this->_query_retrieve_level=="IMAGE" && !fields_builder.hasField("00080018"))
        {
            fields_builder << "00080018" << 1;
        }

        // Handle reduce-related attributes
        std::string reduce_function;
        mongo::BSONObjBuilder initial_builder;

        // Number of XXX Related Instances (0020,120X)
        if(query_dataset.hasField("00201204"))
        {
            this->_instance_count_tag = DCM_NumberOfPatientRelatedInstances;
        }
        else if(query_dataset.hasField("00201208"))
        {
            this->_instance_count_tag = DCM_NumberOfStudyRelatedInstances;
        }
        else if(query_dataset.hasField("00201209"))
        {
            this->_instance_count_tag = DCM_NumberOfSeriesRelatedInstances;
        }
        else
        {
            this->_instance_count_tag = DCM_UndefinedTagKey;
        }
        if (this->_instance_count_tag != DCM_UndefinedTagKey)
        {
            reduce_function += "result.instance_count+=1;";
            initial_builder << "instance_count" << 0;
        }

        // Format the reduce function
        reduce_function = "function(current, result) { " + reduce_function + " }";

        // Perform the DB query.
        mongo::BSONObj const fields = fields_builder.obj();

        mongo::BSONObj group_command = BSON("group" << BSON(
            "ns" << "datasets" << "key" << fields << "cond" << db_query.obj() <<
            "$reduce" << reduce_function << "initial" << initial_builder.obj() 
        ));
        
        DBConnection::get_instance().get_connection().runCommand
            (DBConnection::get_instance().get_db_name(), 
                group_command, this->_info, 0);
                
        this->_results = this->_info["retval"].Array();
        this->_index = 0;

        this->_status = STATUS_Pending;
    } // if (responseCount == 1)
    
    /* only cancel if we have pending responses */
    if (cancelled && this->_status == STATUS_Pending)
    {
        this->cancel();
    }
    
    /* Process next result */
    if (this->_status == STATUS_Pending)
    {
        this->_priority = request->Priority;
        this->next(responseIdentifiers);
    }
    
    /* set response status */
    response->DimseStatus = this->_status;
    *stDetail = NULL;
}

void 
GetResponseGenerator
::next(DcmDataset ** responseIdentifiers)
{
    if(this->_index == this->_results.size())
    {
        // We're done.
        this->_status = STATUS_Success;
    }
    else
    {
        BSONToDataSet bson_to_dataset;
        mongo::BSONObj item = this->_results[this->_index].Obj();
        
        if ( ! item.hasField("location"))
        {
            throw ExceptionPACS("Unable to retrieve location field.");
        }
        
        std::string const path = item.getField("location").String();
        DcmFileFormat fileformat;
        fileformat.loadFile(path.c_str());
        DcmDataset* dataset = fileformat.getAndRemoveDataset();
        
        OFString sopclassuid;
        OFCondition result = dataset->findAndGetOFString(DCM_SOPClassUID, sopclassuid);
        if (result.bad())
        {
            throw ExceptionPACS("Missing SOPClassUID field in dataset.");
        }
        OFString sopinstanceuid;
        result = dataset->findAndGetOFString(DCM_SOPInstanceUID, sopinstanceuid);
        if (result.bad())
        {
            throw ExceptionPACS("Missing SOPInstanceUID field in dataset.");
        }
        
        // Perform sub operation
        result = this->performGetSubOperation(sopclassuid.c_str(), 
                                              sopinstanceuid.c_str(), 
                                              dataset);
        if (result.bad())
        {
            research_pacs::loggerError() << "Get Sub-Op Failed: "
                                         << result.text();
        }
            
        ++this->_index;

        this->_status = STATUS_Pending;
    }
}

OFCondition 
GetResponseGenerator
::performGetSubOperation(const char* sopClassUID, const char* sopInstanceUID,
                         DcmDataset* dataset)
{
    DIC_US msgID = this->_scp->get_association()->nextMsgID++;
    
    /* which presentation context should be used */
    T_ASC_PresentationContextID presID;
    presID = ASC_findAcceptedPresentationContextID(this->_scp->get_association(), 
                                                   sopClassUID);
    if (presID == 0)
    {
        research_pacs::loggerError() << "No presentation context for: "
                                     << dcmSOPClassUIDToModality(sopClassUID, "OT");
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }
    else
    {
        /* make sure that we can send images in this presentation context */
        T_ASC_PresentationContext pc;
        ASC_findAcceptedPresentationContext(this->_scp->get_association()->params,
                                            presID, &pc);
        if (pc.acceptedRole != ASC_SC_ROLE_SCP &&
            pc.acceptedRole != ASC_SC_ROLE_SCUSCP)
        {
            research_pacs::loggerError() << "No presentation context with requestor SCP role for: "
                                         << dcmSOPClassUIDToModality(sopClassUID, "OT");
            return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
        }
    }
    
    // Create a C-Store request
    T_DIMSE_C_StoreRQ req;
    req.MessageID = msgID;
    strcpy(req.AffectedSOPClassUID, sopClassUID);
    strcpy(req.AffectedSOPInstanceUID, sopInstanceUID);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = this->_priority;
    req.opts = 0;
    
    T_DIMSE_DetectedCancelParameters cancelParameters;
    T_DIMSE_C_StoreRSP rsp;
    DcmDataset* stdetail = NULL;
    
    research_pacs::loggerInfo() << "Store SCU RQ: MsgID " << msgID;
    
    // Send the C-Store request
    OFCondition result = DIMSE_storeUser(this->_scp->get_association(), 
                                         presID, &req, NULL, dataset, 
                                         getSubProcessCallback, this,
                                         DIMSE_BLOCKING, 0, &rsp, 
                                         &stdetail, &cancelParameters);
        
    return result;
}
    
} // namespace research_pacs
