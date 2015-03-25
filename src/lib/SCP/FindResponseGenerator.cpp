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
#include "core/DBConnection.h"
#include "core/ExceptionPACS.h"
#include "core/LoggerPACS.h"
#include "core/NetworkPACS.h"
#include "FindResponseGenerator.h"

namespace dopamine
{

FindResponseGenerator
::FindResponseGenerator(FindSCP* scp, std::string const & ouraetitle):
    ResponseGenerator(scp, ouraetitle), // base class initialisation
    _convert_modalities_in_study(false)
{
}

FindResponseGenerator
::~FindResponseGenerator()
{
    // Nothing to do
}

void 
FindResponseGenerator
::callBackHandler(
    /* in */
    OFBool cancelled, T_DIMSE_C_FindRQ* request,
    DcmDataset* requestIdentifiers, int responseCount,
    /* out */
    T_DIMSE_C_FindRSP* response, DcmDataset** responseIdentifiers,
    DcmDataset** stDetail)
{
    if (responseCount == 1)
    {
        // Look for user authorization
        if ( !NetworkPACS::get_instance().check_authorization(
                 this->_scp->get_association()->params->DULparams.reqUserIdentNeg,
                 Service_Query) )
        {
            loggerWarning() << "User not allowed to perform FIND";

            this->_status = STATUS_FIND_Refused_OutOfResources;
            response->DimseStatus = STATUS_FIND_Refused_OutOfResources;

            this->createStatusDetail(STATUS_FIND_Refused_OutOfResources, DCM_UndefinedTagKey,
                                     OFString("User not allowed to perform FIND"), stDetail);

            return;
        }

        this->_status = STATUS_Pending;

        mongo::BSONObj constraint =
                NetworkPACS::get_instance().get_constraint_for_user(
                    this->_scp->get_association()->params->DULparams.reqUserIdentNeg,
                    Service_Query);

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

            // Always include the field in the results
            fields_builder << element.fieldName() << 1;

            std::string const vr = array[0].String();
            mongo::BSONElement const & value = array[1];
            Match::Type const match_type = this->_get_match_type(vr, value);

            DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);
            // Match the array element containing the value
            (this->*function)(std::string(element.fieldName())+".1", vr, value, db_query);
        }

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

            this->_status = STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass;
            response->DimseStatus = STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass;

            this->createStatusDetail(STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass,
                                     DCM_QueryRetrieveLevel, OFString(condition.text()),
                                     stDetail);
            return;
        }

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
        if(this->_instance_count_tag!=DCM_UndefinedTagKey)
        {
            reduce_function += "result.instance_count+=1;";
            initial_builder << "instance_count" << 0;
        }

        // Modalities in Study (0008,0061)
        if(query_dataset.hasField("00080061"))
        {
            // Use the Modality attribute
            mongo::BSONElement modalities = query_dataset["00080061"];
            Match::Type const match_type =
                this->_get_match_type("CS", modalities);
            DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);
            (this->*function)("00080060.1", "CS", modalities, db_query);
            fields_builder << "00080060" << 1;
            reduce_function +=
                "if(result.modalities_in_study.indexOf(current[\"00080060\"][1])==-1) "
                "{ result.modalities_in_study.push(current[\"00080060\"][1]); }";
            initial_builder << "modalities_in_study" << mongo::BSONArrayBuilder().arr();
            this->_convert_modalities_in_study = true;
        }
        else
        {
            this->_convert_modalities_in_study = false;
        }

        mongo::BSONArrayBuilder finalquerybuilder;
        finalquerybuilder << constraint << db_query.obj();
        mongo::BSONObjBuilder finalquery;
        finalquery << "$and" << finalquerybuilder.arr();

        // Format the reduce function
        reduce_function = "function(current, result) { " + reduce_function + " }";

        // Perform the DB query.
        mongo::BSONObj const fields = fields_builder.obj();
        mongo::BSONObj group_command = BSON("group" << BSON(
            "ns" << "datasets" << "key" << fields << "cond" << finalquery.obj() <<
            "$reduce" << reduce_function << "initial" << initial_builder.obj()
        ));

        NetworkPACS::get_instance().get_connection().get_connection().runCommand
            (NetworkPACS::get_instance().get_connection().get_db_name(),
                group_command, this->_info, 0);

        this->_results = this->_info["retval"].Array();

        this->_index = 0;
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
    if(this->_index == this->_results.size())
    {
        // We're done.
        this->_status = STATUS_Success;
    }
    else
    {
        BSONToDataSet bson_to_dataset;
        mongo::BSONObj item = this->_results[this->_index].Obj();

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

            this->createStatusDetail(STATUS_FIND_Failed_UnableToProcess,
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

                this->createStatusDetail(STATUS_FIND_Failed_UnableToProcess,
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

                this->createStatusDetail(STATUS_FIND_Failed_UnableToProcess,
                                         DCM_ModalitiesInStudy,
                                         OFString(condition.text()), details);
                return;
            }
        }

        ++this->_index;

        this->_status = STATUS_Pending;
    }
}

} // namespace dopamine
