/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/
#include <inttypes.h>
#include <boost/filesystem.hpp>

#include "ConverterBSON/DataSetToBSON.h"
#include "ConverterBSON/IsPrivateTag.h"
#include "ConverterBSON/VRMatch.h"
#include "core/ConfigurationPACS.h"
#include "core/DBConnection.h"
#include "StoreSCP.h"

namespace research_pacs
{
    
template<typename TIterator>
uint32_t hashCode(TIterator begin, TIterator end)
{
    uint32_t hash=0;
    TIterator it(begin);
    while(it != end)
    {   
        hash = 31*hash+(*it);
        ++it;
    }   
    return hash;
}

template<typename TString>
uint32_t hashCode(TString const & s)
{
    char const * const begin = s.c_str();
    char const * const end = begin+s.size();
    return hashCode(begin, end);
}

std::string hashToString(uint32_t hash)
{
    char temp[9]; // Use one more char for '\0'
    memset(&temp[0], 0, sizeof(temp)); // Set all to '\0'
    snprintf(&temp[0], sizeof(temp), "%08X" PRIX32, hash);
    memset(&temp[8], 0, 1); // make sure last char is '\0'
    return std::string(temp);
}

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
        StoreSCP * scp = reinterpret_cast<StoreCallbackData*>(callbackData)->scp;

        // Check if we already have this dataset, based on its SOP Instance UID
        OFString sop_instance_uid;
        (*imageDataSet)->findAndGetOFString(DcmTagKey(0x0008,0x0018), sop_instance_uid);
        
        mongo::auto_ptr<mongo::DBClientCursor> cursor =
            DBConnection::get_instance().get_connection().query(
                DBConnection::get_instance().get_db_name()+"."+"datasets",
                QUERY("00080018" << sop_instance_uid.c_str()));

        if(cursor->more())
        {
            // We already have this SOP Instance UID, do not store it
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
            // DCM4CHEE does:
            // /root
            //   /year/month/day (of now)
            //   /(32-bits hash of Study Instance UID
            //   /(32-bits hash of Series Instance UID)
            //   /(32-bits hash of SOP Instance UID)
            // The 32-bits hash is based on String.hashCode (http://docs.oracle.com/javase/1.5.0/docs/api/java/lang/String.html#hashCode%28%29)

            boost::gregorian::date const today(
                boost::gregorian::day_clock::universal_day());

            OFString study_instance_uid;
            (*imageDataSet)->findAndGetOFStringArray(DCM_StudyInstanceUID, study_instance_uid);
            OFString series_instance_uid;
            (*imageDataSet)->findAndGetOFStringArray(DCM_SeriesInstanceUID, series_instance_uid);
            OFString sop_instance_uid;
            (*imageDataSet)->findAndGetOFStringArray(DCM_SOPInstanceUID, sop_instance_uid);

            std::string const study_hash = hashToString(hashCode(study_instance_uid));
            std::string const series_hash = hashToString(hashCode(series_instance_uid));
            std::string const sop_instance_hash = hashToString(hashCode(sop_instance_uid));

            boost::filesystem::path const destination =
                boost::filesystem::path(ConfigurationPACS::get_instance().GetValue("dicom.storage_path"))
                    /boost::lexical_cast<std::string>(today.year())
                    /boost::lexical_cast<std::string>(today.month().as_number())
                    /boost::lexical_cast<std::string>(today.day())
                    /study_hash/series_hash/sop_instance_hash;

            // Store the dataset in DB and in filesystem
            builder << "location" << destination.string();
            DBConnection::get_instance().get_connection().insert(DBConnection::get_instance().get_db_name()+".datasets", builder.obj());
            boost::filesystem::create_directories(destination.parent_path());
            
            file_format.saveFile(destination.string().c_str(), EXS_LittleEndianExplicit);
        }
    }
}

StoreSCP
::StoreSCP(T_ASC_Association * assoc, 
           T_ASC_PresentationContextID presID, 
           T_DIMSE_C_StoreRQ * req):
    SCP(assoc, presID), _request(req)
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
    std::cout << "Received Store SCP: MsgID " 
              << this->_request->MessageID << std::endl;
              
    StoreCallbackData data;
    data.scp = this;

    if (!dcmIsaStorageSOPClassUID(this->_request->AffectedSOPClassUID))
    {
        /* callback will send back sop class not supported status */
        data.status = STATUS_STORE_Refused_SOPClassNotSupported;
    }

    // store SourceApplicationEntityTitle in metaheader
    if (this->_association && this->_association->params)
    {
        char const * aet = this->_association->params->DULparams.callingAPTitle;
        if(aet != NULL)
        {
            data.source_application_entity_title = this->_association->params->DULparams.callingAPTitle;
        }
    }

    /* we must still retrieve the data set even if some error has occured */
    DcmDataset dset;
    DcmDataset * dset_ptr = &dset;
    return DIMSE_storeProvider(this->_association, this->_presentationID, 
                               this->_request, (char *)NULL, 1,
                               &dset_ptr, storeCallback, (void*)&data,
                               DIMSE_BLOCKING, 0);
}

} // namespace research_pacs
