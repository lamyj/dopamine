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
#include "core/NetworkPACS.h"
#include "core/ExceptionPACS.h"
#include "core/LoggerPACS.h"
#include "GetResponseGenerator.h"

namespace dopamine
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
        OFCondition condition = requestIdentifiers->findAndGetOFString(DCM_QueryRetrieveLevel,
                                                                       ofstring);
        if (condition.bad())
        {
            dopamine::loggerError() << "Cannot find DCM_QueryRetrieveLevel: "
                                    << condition .text();

            this->_status = STATUS_GET_Failed_IdentifierDoesNotMatchSOPClass;
            response->DimseStatus = STATUS_GET_Failed_IdentifierDoesNotMatchSOPClass;

            this->createStatusDetail(STATUS_GET_Failed_IdentifierDoesNotMatchSOPClass,
                                     DCM_QueryRetrieveLevel, OFString(condition.text()), stDetail);
            return;
        }

        this->_query_retrieve_level = std::string(ofstring.c_str());
        if (!fields_builder.hasField("00100020"))
        {
            fields_builder << "00100020" << 1;
        }
        if ((this->_query_retrieve_level=="STUDY" || this->_query_retrieve_level=="SERIES" ||
             this->_query_retrieve_level=="IMAGE") &&
            !fields_builder.hasField("0020000d"))
        {
            fields_builder << "0020000d" << 1;
        }
        if ((this->_query_retrieve_level=="SERIES" || this->_query_retrieve_level=="IMAGE") &&
            !fields_builder.hasField("0020000e"))
        {
            fields_builder << "0020000e" << 1;
        }
        if (this->_query_retrieve_level=="IMAGE" && !fields_builder.hasField("00080018"))
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
        
        NetworkPACS::get_instance().get_connection().get_connection().runCommand
            (NetworkPACS::get_instance().get_connection().get_db_name(),
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
        this->next(responseIdentifiers, stDetail);
    }
    
    /* set response status */
    response->DimseStatus = this->_status;
    if (this->_status == STATUS_Pending || this->_status == STATUS_Success)
    {
        (*stDetail) = NULL;
    }
}

void 
GetResponseGenerator
::next(DcmDataset ** responseIdentifiers, DcmDataset **details)
{
    if(this->_index == this->_results.size())
    {
        // We're done.
        this->_status = STATUS_Success;
    }
    else
    {
        mongo::BSONObj item = this->_results[this->_index].Obj();
        
        if ( ! item.hasField("location"))
        {
            dopamine::loggerError() << "Unable to retrieve location field.";

            this->createStatusDetail(STATUS_GET_Failed_UnableToProcess,
                                     DCM_UndefinedTagKey, OFString(EC_CorruptedData.text()), details);

            this->_status = STATUS_GET_Failed_UnableToProcess;
            return;
        }
        
        std::string const path = item.getField("location").String();
        DcmFileFormat fileformat;
        OFCondition condition = fileformat.loadFile(path.c_str());
        if (condition.bad())
        {
            dopamine::loggerError() << "Unable to load file: " << condition.text();

            this->createStatusDetail(STATUS_GET_Failed_UnableToProcess,
                                     DCM_UndefinedTagKey, OFString(condition.text()), details);

            this->_status = STATUS_GET_Failed_UnableToProcess;
            return;
        }
        DcmDataset* dataset = fileformat.getAndRemoveDataset();
        
        OFString sopclassuid;
        condition = dataset->findAndGetOFString(DCM_SOPClassUID, sopclassuid);
        if (condition.bad())
        {
            dopamine::loggerError() << "Missing SOPClassUID field in dataset: "
                                    << condition.text();

            this->createStatusDetail(STATUS_GET_Failed_UnableToProcess,
                                     DCM_SOPClassUID, OFString(condition.text()), details);

            this->_status = STATUS_GET_Failed_UnableToProcess;
            return;
        }
        OFString sopinstanceuid;
        condition = dataset->findAndGetOFString(DCM_SOPInstanceUID, sopinstanceuid);
        if (condition.bad())
        {
            dopamine::loggerError() << "Missing DCM_SOPInstanceUID field in dataset: "
                                    << condition.text();

            this->createStatusDetail(STATUS_GET_Failed_UnableToProcess,
                                     DCM_SOPInstanceUID, OFString(condition.text()), details);

            this->_status = STATUS_GET_Failed_UnableToProcess;
            return;
        }

        // Perform sub operation
        condition = this->performGetSubOperation(sopclassuid.c_str(),
                                                 sopinstanceuid.c_str(),
                                                 dataset);
        if (condition.bad())
        {
            dopamine::loggerError() << "Get Sub-Op Failed: "
                                         << condition.text();

            this->createStatusDetail(STATUS_GET_Failed_UnableToProcess,
                                     DCM_UndefinedTagKey, OFString(condition.text()), details);

            this->_status = STATUS_GET_Failed_UnableToProcess;
            return;
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
        dopamine::loggerError() << "No presentation context for: "
                                << dcmSOPClassUIDToModality(sopClassUID, "OT");
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }
    else
    {
        /* make sure that we can send images in this presentation context */
        T_ASC_PresentationContext pc;
        OFCondition condition =
                ASC_findAcceptedPresentationContext(this->_scp->get_association()->params,
                                                    presID, &pc);
        if (condition.bad())
        {
            return condition;
        }

        if (pc.acceptedRole != ASC_SC_ROLE_SCP &&
            pc.acceptedRole != ASC_SC_ROLE_SCUSCP)
        {
            dopamine::loggerError() << "No presentation context with requestor SCP role for: "
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
    
    dopamine::loggerInfo() << "Store SCU RQ: MsgID " << msgID;

    // Send the C-Store request
    return DIMSE_storeUser(this->_scp->get_association(), presID, &req, NULL,
                           dataset, getSubProcessCallback, this,
                           DIMSE_BLOCKING, 30, &rsp, &stdetail,
                           &cancelParameters);
}
    
} // namespace dopamine
