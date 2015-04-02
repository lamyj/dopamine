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
#include "services/ServicesTools.h"
#include "StoreResponseGenerator.h"

namespace dopamine
{

namespace services
{

StoreResponseGenerator
::StoreResponseGenerator(T_ASC_Association *request_association):
    ResponseGenerator(request_association), _destination_path("")
{
    // Nothing to do
}

StoreResponseGenerator
::~StoreResponseGenerator()
{
    // Nothing to do
}

void
StoreResponseGenerator
::process(T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ *req,
          char *imageFileName, DcmDataset **imageDataSet,
          T_DIMSE_C_StoreRSP *rsp, DcmDataset **stDetail)
{
    if(progress->state == DIMSE_StoreEnd)
    {
        Uint16 result = this->set_query(*imageDataSet);

        if (result != STATUS_Success)
        {
            rsp->DimseStatus = result;
            createStatusDetail(result, DCM_UndefinedTagKey,
                               OFString("An error occured while processing Storage"),
                               stDetail);
        }
    }
}

Uint16 StoreResponseGenerator::set_query(DcmDataset * dataset)
{
    if (this->_connection.isFailed())
    {
        loggerWarning() << "Could not connect to database: " << this->_db_name;
        return STATUS_STORE_Refused_OutOfResources;
    }

    // Look for user authorization
    std::string const username = get_username(this->_request_association->params->DULparams.reqUserIdentNeg);
    if ( ! is_authorized(this->_connection, this->_db_name, username, Service_Store) )
    {
        loggerWarning() << "User not allowed to perform STORE";
        return STATUS_STORE_Refused_OutOfResources;
    }

    // Check if we already have this dataset, based on its SOP Instance UID
    OFString sop_instance_uid;
    OFCondition ret = dataset->findAndGetOFString(DcmTagKey(0x0008,0x0018),
                                                  sop_instance_uid);
    if (ret.bad())
    {
        loggerWarning() << "Cannot retrieve SOP Instance UID";
        return STATUS_STORE_Warning_CoersionOfDataElements;
    }

    mongo::BSONObj group_command =
            BSON("count" << "datasets" << "query" << BSON("00080018" << sop_instance_uid.c_str()));

    mongo::BSONObj info;
    this->_connection.runCommand(this->_db_name, group_command, info, 0);

    // If the command correctly executed and database entries match
    if (info["ok"].Double() == 1 && info["n"].Double() > 0)
    {
        // We already have this SOP Instance UID, do not store it
        loggerWarning() << "Store: SOP Instance UID already register";
        return STATUS_STORE_Refused_OutOfResources;
    }
    else if (info["ok"].Double() != 1)
    {
        loggerWarning() << "Could not connect to database: " << this->_db_name;
        return STATUS_STORE_Refused_OutOfResources;
    }
    else
    {
        this->create_destination_path(dataset);

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
        converter(dataset, builder);

        // Store it in the Mongo DB instance
        mongo::OID const id(mongo::OID::gen());
        builder << "_id" << id;

        // Add DICOM file path into BSON object
        builder << "location" << this->_destination_path;

        // Check user's constraints (user's Rights)
        mongo::BSONObj bsondataset = builder.obj();
        if (!this->is_dataset_allowed_for_storage(bsondataset))
        {
            loggerWarning() << "User not allowed to perform STORE";
            return STATUS_STORE_Refused_OutOfResources;
        }

        // Store the dataset in DB
        std::stringstream stream;
        stream << this->_db_name << ".datasets";
        this->_connection.insert(stream.str(), bsondataset);

        std::string callingaptitle = "";
        char const * aet = this->_request_association->params->DULparams.callingAPTitle;
        if(aet != NULL)
        {
            callingaptitle = this->_request_association->params->DULparams.callingAPTitle;
        }

        // Create the header of the new file
        DcmFileFormat file_format(dataset);
        file_format.getMetaInfo()->putAndInsertString(DCM_SourceApplicationEntityTitle,
                                                      callingaptitle.c_str());

        // Create DICOM file
        boost::filesystem::create_directories(boost::filesystem::path(this->_destination_path).parent_path());
        file_format.saveFile(this->_destination_path.c_str(),
                             EXS_LittleEndianExplicit);
    }

    return STATUS_Success;
}

void
StoreResponseGenerator
::create_destination_path(DcmDataset * dataset)
{
    // Compute the destination filename
    boost::gregorian::date const today(
        boost::gregorian::day_clock::universal_day());

    OFString study_instance_uid;
    dataset->findAndGetOFStringArray(DCM_StudyInstanceUID, study_instance_uid);
    OFString series_instance_uid;
    dataset->findAndGetOFStringArray(DCM_SeriesInstanceUID, series_instance_uid);
    OFString sop_instance_uid;
    dataset->findAndGetOFStringArray(DCM_SOPInstanceUID, sop_instance_uid);

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

    _destination_path = destination.string();
}

bool StoreResponseGenerator
::is_dataset_allowed_for_storage(mongo::BSONObj const & dataset)
{
    std::string const username =
            get_username(this->_request_association->params->DULparams.reqUserIdentNeg);

    // Retrieve user's Rights
    mongo::BSONObj constraint =
            get_constraint_for_user(this->_connection, this->_db_name,
                                    username, Service_Store);

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

} // namespace services

} // namespace dopamine
