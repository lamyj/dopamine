/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "ConverterBSON/DataSetToBSON.h"
#include "ConverterBSON/IsPrivateTag.h"
#include "ConverterBSON/VRMatch.h"
#include "core/ConfigurationPACS.h"
#include "core/Hashcode.h"
#include "core/LoggerPACS.h"
#include "core/NetworkPACS.h"
#include "ResponseGenerator.h"
#include "StoreSCP.h"

namespace dopamine
{

static std::string bsonelement_to_string(mongo::BSONElement const & bsonelement)
{
    std::stringstream builder;
    switch (bsonelement.type())
    {
    case mongo::BSONType::NumberDouble:
    {
        builder << bsonelement.numberDouble();
        break;
    }
    case mongo::BSONType::String:
    {
        builder << bsonelement.String();
        break;
    }
    case mongo::BSONType::NumberInt:
    {
        builder << bsonelement.Int();
        break;
    }
    case mongo::BSONType::NumberLong:
    {
        builder << bsonelement.Long();
        break;
    }
    default:
    {
        builder << bsonelement.toString(false);
        break;
    }
    }

    return builder.str();
}

static bool check_user_authorization(mongo::BSONObj const & dataset,
                                     UserIdentityNegotiationSubItemRQ * identity)
{
    // Retrieve user's Rights
    mongo::BSONObj constraint =
            NetworkPACS::get_instance().get_constraint_for_user(
                identity, Service_Store);

    if (constraint.isEmpty())
    {
        // No constraint
        return true;
    }

    // Compare with input dataset

    // constraint is a Logical OR array
    for (auto itemor : constraint["$or"].Array())
    {
        bool result = true;
        // Each item is a Logical AND array
        for (auto itemand : itemor["$and"].Array())
        {
            // Foreach object
            for(mongo::BSONObj::iterator it=itemand.Obj().begin(); it.more();)
            {
                mongo::BSONElement const element = it.next();

                // Retrieve DICOM field name
                std::string name(element.fieldName());
                name = name.substr(0, 8);

                // Check if input dataset as this field
                if (!dataset.hasField(name))
                {
                    result = false;
                    break;
                }
                else
                {
                    // Compare the field's values
                    std::string valuestr = bsonelement_to_string(dataset.getField(name).Array()[1]);

                    if (element.type() == mongo::BSONType::RegEx)
                    {
                        std::string regex(element.regex());
                        if (!boost::regex_match(valuestr.c_str(), boost::regex(regex.c_str())))
                        {
                            result = false;
                            break;
                        }
                    }
                    else
                    {
                        std::string elementstr = bsonelement_to_string(element);
                        if (valuestr != elementstr)
                        {
                            result = false;
                            break;
                        }
                    }
                }
            }
        }

        // Stop if find dataset match with one condition
        if (result)
        {
            return true;
        }
        // else continue
    }

    return false;
}

/**
 * Callback handler called by the DIMSE_storeProvider callback function
 * @param callbackdata: Callback context (in)
 * @param progress: progress state (in)
 * @param req: original store request (in)
 * @param imageFileName: being received into (in)
 * @param imageDataSet: being received into (in)
 * @param rsp: final store response (out)
 * @param stDetail: status detail for find response (out)
 */
static void storeCallback(
    /* in */
    void *callbackData,
    T_DIMSE_StoreProgress *progress,    /* progress state */
    T_DIMSE_C_StoreRQ *req,             /* original store request */
    char *imageFileName,                /* being received into */
    DcmDataset **imageDataSet,          /* being received into */
    /* out */
    T_DIMSE_C_StoreRSP *rsp,            /* final store response */
    DcmDataset **stDetail)
{
    if(progress->state == DIMSE_StoreEnd)
    {
        // Look for user authorization
        if ( !NetworkPACS::get_instance().check_authorization(
                 reinterpret_cast<StoreCallbackData*>(callbackData)->user_identity,
                 Service_Store) )
        {
            loggerWarning() << "User not allowed to perform STORE";

            rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;

            ResponseGenerator::createStatusDetail(STATUS_STORE_Refused_OutOfResources,
                                                  DCM_UndefinedTagKey,
                                                  OFString("User not allowed to perform STORE"),
                                                  stDetail);

            return;
        }

        // Check if we already have this dataset, based on its SOP Instance UID
        OFString sop_instance_uid;
        (*imageDataSet)->findAndGetOFString(DcmTagKey(0x0008,0x0018), sop_instance_uid);
        
        mongo::auto_ptr<mongo::DBClientCursor> cursor =
            NetworkPACS::get_instance().get_connection().query(
                NetworkPACS::get_instance().get_db_name()+"."+"datasets",
                QUERY("00080018" << sop_instance_uid.c_str()));

        if(cursor->more())
        {
            // We already have this SOP Instance UID, do not store it
            dopamine::loggerWarning() << "Store: SOP Instance UID already register";
        }
        else
        {
            // Convert the dcmtk dataset to BSON
            DataSetToBSON converter;

            converter.get_filters().push_back(std::make_pair(
                IsPrivateTag::New(), DataSetToBSON::FilterAction::EXCLUDE));
            converter.get_filters().push_back(std::make_pair(
                VRMatch::New(EVR_OB), DataSetToBSON::FilterAction::EXCLUDE));
            converter.get_filters().push_back(std::make_pair(
                VRMatch::New(EVR_OF), DataSetToBSON::FilterAction::EXCLUDE));
            converter.get_filters().push_back(std::make_pair(
                VRMatch::New(EVR_OW), DataSetToBSON::FilterAction::EXCLUDE));
            converter.get_filters().push_back(std::make_pair(
                VRMatch::New(EVR_UN), DataSetToBSON::FilterAction::EXCLUDE));
            converter.set_default_filter(DataSetToBSON::FilterAction::INCLUDE);

            mongo::BSONObjBuilder builder;
            converter(*imageDataSet, builder);

            // Store it in the Mongo DB instance
            mongo::OID const id(mongo::OID::gen());
            builder << "_id" << id;

            // Create the header of the new file
            DcmFileFormat file_format(*imageDataSet);
            file_format.getMetaInfo()->putAndInsertString(DCM_SourceApplicationEntityTitle, 
               reinterpret_cast<StoreCallbackData*>(callbackData)->source_application_entity_title.c_str());

            // Compute the destination filename
            boost::gregorian::date const today(
                boost::gregorian::day_clock::universal_day());

            OFString study_instance_uid;
            (*imageDataSet)->findAndGetOFStringArray(DCM_StudyInstanceUID, study_instance_uid);
            OFString series_instance_uid;
            (*imageDataSet)->findAndGetOFStringArray(DCM_SeriesInstanceUID, series_instance_uid);
            OFString sop_instance_uid;
            (*imageDataSet)->findAndGetOFStringArray(DCM_SOPInstanceUID, sop_instance_uid);

            std::string const study_hash = dopamine::hashcode::hashToString
                    (dopamine::hashcode::hashCode(study_instance_uid));
            std::string const series_hash = dopamine::hashcode::hashToString
                    (dopamine::hashcode::hashCode(series_instance_uid));
            std::string const sop_instance_hash = dopamine::hashcode::hashToString
                    (dopamine::hashcode::hashCode(sop_instance_uid));

            boost::filesystem::path const destination =
                boost::filesystem::path(
                    ConfigurationPACS::get_instance().GetValue("dicom.storage_path"))
                    /boost::lexical_cast<std::string>(today.year())
                    /boost::lexical_cast<std::string>(today.month().as_number())
                    /boost::lexical_cast<std::string>(today.day())
                    /study_hash/series_hash/sop_instance_hash;

            // Add DICOM file path into BSON object
            builder << "location" << destination.string();

            // Check user's constraints (user's Rights)
            mongo::BSONObj dataset = builder.obj();
            if (!check_user_authorization(dataset, reinterpret_cast<StoreCallbackData*>(callbackData)->user_identity))
            {
                loggerWarning() << "User not allowed to perform STORE";

                rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;

                ResponseGenerator::createStatusDetail(STATUS_STORE_Refused_OutOfResources,
                                                      DCM_UndefinedTagKey,
                                                      OFString("User not allowed to perform STORE"),
                                                      stDetail);

                return;
            }

            // Store the dataset in DB
            NetworkPACS::get_instance().get_connection().insert(
                NetworkPACS::get_instance().get_db_name()+".datasets",
                dataset);

            // Create DICOM file
            boost::filesystem::create_directories(destination.parent_path());
            file_format.saveFile(destination.string().c_str(), 
                                 EXS_LittleEndianExplicit);
        }
    }
}

StoreSCP
::StoreSCP(T_ASC_Association * assoc, 
           T_ASC_PresentationContextID presID, 
           T_DIMSE_C_StoreRQ * req):
    SCP(assoc, presID), _request(req) // base class initialisation
{
    // nothing to do
}

StoreSCP
::~StoreSCP()
{
    // nothing to do
}

OFCondition 
StoreSCP
::process()
{
    dopamine::loggerInfo() << "Received Store SCP: MsgID "
                                << this->_request->MessageID;
              
    StoreCallbackData data;

    if (!dcmIsaStorageSOPClassUID(this->_request->AffectedSOPClassUID))
    {
        /* callback will send back sop class not supported status */
        data.status = STATUS_STORE_Refused_SOPClassNotSupported;
    }

    // store SourceApplicationEntityTitle in metaheader
    if (this->_association && this->get_association()->params)
    {
        char const * aet = this->_association->params->DULparams.callingAPTitle;
        if(aet != NULL)
        {
            data.source_application_entity_title = 
                this->_association->params->DULparams.callingAPTitle;
        }
    }

    data.user_identity = this->_association->params->DULparams.reqUserIdentNeg;

    /* we must still retrieve the data set even if some error has occured */
    DcmDataset dset;
    DcmDataset * dset_ptr = &dset;
    return DIMSE_storeProvider(this->_association, this->_presentationID, 
                               this->_request, (char *)NULL, 1,
                               &dset_ptr, storeCallback, (void*)&data,
                               DIMSE_BLOCKING, 0);
}

} // namespace dopamine
