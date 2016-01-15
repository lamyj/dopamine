/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/regex.hpp>

#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

#include "ConverterBSON/Dataset/BSONToDataSet.h"
#include "ConverterBSON/Dataset/DataSetToBSON.h"
#include "ConverterBSON/Dataset/IsPrivateTag.h"
#include "ConverterBSON/Dataset/VRMatch.h"
#include "core/ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "ServicesTools.h"

#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcistrmb.h>

namespace dopamine
{

namespace services
{

bool create_db_connection(DataBaseInformation & db_information)
{
    // Get all indexes
    std::string const indexlist =
            ConfigurationPACS::get_instance().get_value("database.indexlist");
    std::vector<std::string> indexeslist;
    boost::split(indexeslist, indexlist, boost::is_any_of(";"));

    db_information.db_name =
        ConfigurationPACS::get_instance().get_value("database.dbname");
    db_information.bulk_data =
        ConfigurationPACS::get_instance().get_value("database.bulk_data");
    if(db_information.bulk_data.empty())
    {
        db_information.bulk_data = db_information.db_name;
    }
    std::string const db_host =
            ConfigurationPACS::get_instance().get_value("database.hostname");
    std::string const db_port =
            ConfigurationPACS::get_instance().get_value("database.port");

    if(db_information.db_name == "" || db_host == "" || db_port == "")
    {
        return false;
    }

    // Try to connect database
    // Disconnect is automatic when it calls the destructors
    std::string errormsg = "";
    if(!db_information.connection.connect(
        mongo::HostAndPort(db_host + ":" + db_port), errormsg))
    {
        return false;
    }

    // Create indexes
    std::string const datasets = db_information.db_name + ".datasets";

    for (auto currentIndex : indexeslist)
    {
        DcmTag dcmtag;
        OFCondition condition = DcmTag::findTagFromName(currentIndex.c_str(),
                                                        dcmtag);

        if (condition.good())
        {
            // convert Uint16 => string XXXXYYYY
            char buffer[9];
            snprintf(buffer, 9, "%04x%04x",
                     dcmtag.getGroup(), dcmtag.getElement());

            std::stringstream stream;
            stream << "\"" << buffer << "\"";

            db_information.connection.ensureIndex(
                datasets, BSON(stream.str() << 1), false,
                std::string(dcmtag.getTagName())
            );
        }
    }

    if(db_information.db_name != db_information.bulk_data)
    {
        db_information.connection.ensureIndex(
            db_information.bulk_data+".datasets", BSON("SOPInstanceUID" << 1));
    }

    return true;
}

void
create_status_detail(Uint16 const & errorCode, DcmTagKey const & key,
                     OFString const & comment, DcmDataset **statusDetail)
{
    DcmElement * element;
    std::vector<Uint16> vect;

    (*statusDetail) = new DcmDataset();

    OFCondition condition = (*statusDetail)->insertEmptyElement(DCM_Status);
    if (condition.bad())
    {
        throw ExceptionPACS(condition.text());
    }
    condition = (*statusDetail)->findAndGetElement(DCM_Status, element);
    if (condition.bad())
    {
        throw ExceptionPACS(condition.text());
    }
    vect.clear();
    vect.push_back(errorCode);
    condition = element->putUint16Array(&vect[0], vect.size());
    if (condition.bad())
    {
        throw ExceptionPACS(condition.text());
    }

    condition = (*statusDetail)->insertEmptyElement(DCM_OffendingElement);
    if (condition.bad())
    {
        throw ExceptionPACS(condition.text());
    }
    condition = (*statusDetail)->findAndGetElement(DCM_OffendingElement,
                                                   element);
    if (condition.bad())
    {
        throw ExceptionPACS(condition.text());
    }
    vect.clear();
    vect.push_back(key.getGroup());
    vect.push_back(key.getElement());
    condition = element->putUint16Array(&vect[0], vect.size()/2);
    if (condition.bad())
    {
        throw ExceptionPACS(condition.text());
    }

    condition = (*statusDetail)->putAndInsertOFStringArray(DCM_ErrorComment,
                                                           comment);
    if (condition.bad())
    {
        throw ExceptionPACS(condition.text());
    }
}

std::string
get_username(UserIdentityNegotiationSubItemRQ *userIdentNeg)
{
    std::string lcurrentUser = "";

    // Retrieve UserName for Identity Type: User or User/Pasword
    if ((userIdentNeg != NULL) &&
        (userIdentNeg->getIdentityType() == ASC_USER_IDENTITY_USER ||
         userIdentNeg->getIdentityType() == ASC_USER_IDENTITY_USER_PASSWORD))
    {
        // Get user name
        char * user;
        Uint16 user_length;
        userIdentNeg->getPrimField(user, user_length);
        // user is not NULL-terminated
        lcurrentUser = std::string(user, user_length);
        delete [] user;
    }

    return lcurrentUser;
}

bool
is_authorized(DataBaseInformation & db_information,
              std::string const & username,
              std::string const & servicename)
{
    mongo::BSONArrayBuilder builder;
    if (username != "")
    {
        builder << "*";
    }
    builder << username;

    mongo::BSONObjBuilder fields_builder;
    fields_builder << "principal_name" << BSON("$in" << builder.arr())
                   << "service" << BSON("$in" << BSON_ARRAY(Service_All <<
                                                            servicename));

    mongo::BSONObj const group_command = BSON("count" << "authorization" <<
                                              "query" << fields_builder.obj());

    mongo::BSONObj info;
    db_information.connection.runCommand(
        db_information.db_name, group_command, info, 0);

    // If the command correctly executed and database entries match
    if (info["ok"].Double() == 1 && info["n"].Double() > 0)
    {
        return true;
    }

    // Not allowed
    return false;
}

mongo::BSONObj
get_constraint_for_user(DataBaseInformation & db_information,
                        std::string const & username,
                        std::string const & servicename)
{
    mongo::BSONArrayBuilder builder;
    if (username != "")
    {
        builder << "*";
    }
    builder << username;

    // Create Query with user's authorization
    mongo::BSONObjBuilder fields_builder;
    fields_builder << "principal_name" << BSON("$in" << builder.arr())
                   << "service" << BSON("$in" << BSON_ARRAY(Service_All <<
                                                            servicename));
    mongo::BSONObjBuilder initial_builder;
    mongo::BSONObj const group_command =
            BSON("group" << BSON("ns" << "authorization" <<
                                 "key" << BSON("dataset" << 1) <<
                                 "cond" << fields_builder.obj() <<
                                 "$reduce" << "function(current, result) { }" <<
                                 "initial" << initial_builder.obj()
    ));

    mongo::BSONObj result;
    db_information.connection.runCommand(
        db_information.db_name, group_command, result, 0);

    if (result["ok"].Double() != 1 || result["count"].Double() == 0)
    {
        // Create a constraint to deny all actions
        mongo::BSONObjBuilder builder_none;
        builder_none << "00080018.Value"
                     << BSON_ARRAY("_db8eeea6_e0eb_48b8_9a02_a94926b76992");

        return builder_none.obj();
    }

    mongo::BSONArrayBuilder constaintarray;
    for (auto item : result["retval"].Array())
    {
        // if dataset: "" or dataset: {} => all is allowed
        if ((item["dataset"].type() == mongo::BSONType::String &&
             item["dataset"].String() == "") ||
            (item["dataset"].type() == mongo::BSONType::Object &&
             item["dataset"].Obj().isEmpty()))
        {
            // No constraint
            return mongo::BSONObj();
        }

        /*
         * Warning: throw an exception if
         * item["dataset"].type() != mongo::BSONType::Object
         */

        mongo::BSONArrayBuilder andarray;
        for(mongo::BSONObj::iterator it = item["dataset"].Obj().begin();
            it.more();)
        {
            mongo::BSONElement const element = it.next();

            mongo::BSONObjBuilder object;
            if (element.type() == mongo::BSONType::RegEx)
            {
                object.appendRegex(std::string(element.fieldName())+".Value",
                                   element.regex(), "");
            }
            else
            {
                object << std::string(element.fieldName())+".Value"
                       << bsonelement_to_string(element);
            }
            andarray << object.obj();
        }
        mongo::BSONObjBuilder andobject;
        andobject << "$and" << andarray.arr();
        constaintarray << andobject.obj();
    }
    mongo::BSONObjBuilder constraint;
    constraint << "$or" << constaintarray.arr();

    return constraint.obj();
}

std::string
bsonelement_to_string(mongo::BSONElement const & bsonelement)
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

std::string
replace(std::string const & value, std::string const & old,
        std::string const & new_)
{
    std::string result(value);
    size_t begin=0;
    while(std::string::npos != (begin=result.find(old, begin)))
    {
        result = result.replace(begin, old.size(), new_);
        begin = (begin+new_.size()<result.size())?begin+new_.size()
                                                 :std::string::npos;
    }

    return result;
}

mongo::BSONObj
dataset_to_bson(DcmDataset * const dataset, bool isforstorage)
{
    // Convert the dataset to BSON, excluding Query/Retrieve Level.
    converterBSON::DataSetToBSON dataset_to_bson;
    if (isforstorage)
    {
        dataset_to_bson.get_filters().push_back(std::make_pair(
            converterBSON::IsPrivateTag::New(),
            converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            converterBSON::VRMatch::New(EVR_OB),
            converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            converterBSON::VRMatch::New(EVR_OF),
            converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            converterBSON::VRMatch::New(EVR_OW),
            converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            converterBSON::VRMatch::New(EVR_UN),
            converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
    }
    dataset_to_bson.set_default_filter(
                converterBSON::DataSetToBSON::FilterAction::INCLUDE);

    return dataset_to_bson.from_dataset(dataset);
}

DcmDataset *
bson_to_dataset(DataBaseInformation & db_information,
                mongo::BSONObj object)
{
    DcmDataset* dataset = NULL;

    if ( ! object.hasField("Content"))
    {
        converterBSON::BSONToDataSet bson2dataset;
        DcmDataset result = bson2dataset.to_dataset(object);
        dataset = new DcmDataset(result);
    }
    else
    {
        std::string value = get_dataset_as_string(db_information, object);

        // Create buffer for DCMTK
        DcmInputBufferStream* inputbufferstream = new DcmInputBufferStream();
        inputbufferstream->setBuffer(value.c_str(), value.size());
        inputbufferstream->setEos();

        // Convert buffer into Dataset
        DcmFileFormat fileformat;
        fileformat.transferInit();
        OFCondition condition = fileformat.read(*inputbufferstream);
        fileformat.transferEnd();

        delete inputbufferstream;

        if (condition.bad())
        {
            std::stringstream streamerror;
            streamerror << "Cannot read dataset: " << condition.text();
            throw ExceptionPACS(streamerror.str());
        }

        dataset = fileformat.getAndRemoveDataset();
    }

    return dataset;
}

database_status
insert_dataset(DataBaseInformation & db_information,
               std::string const & username,
               DcmDataset *dataset,
               std::string const & callingaet)
{
    // Convert Dataset into BSON object
    mongo::BSONObj object = dataset_to_bson(dataset, true);
    if (!object.isValid() || object.isEmpty())
    {
        return CONVERSION_ERROR;
    }

    // Check user's constraints (user's Rights)
    if (!is_dataset_allowed_for_storage(db_information, username, object))
    {
        return NOT_ALLOW;
    }

    // Convert the dataset to its binary representation
    std::string buffer;
    {
        // Create the header of the new file
        DcmFileFormat file_format(dataset);
        // Keep the original transfer syntax (if available)
        E_TransferSyntax xfer = dataset->getOriginalXfer();
        if (xfer == EXS_Unknown)
        {
          // No information about the original transfer syntax: This is
          // most probably a DICOM dataset that was read from memory.
          xfer = EXS_LittleEndianExplicit;
        }
        file_format.getMetaInfo()->putAndInsertString(
                    DCM_SourceApplicationEntityTitle, callingaet.c_str());
        OFCondition condition = file_format.validateMetaInfo(xfer);
        if (condition.bad())
        {
            return CONVERSION_ERROR;
        }
        file_format.removeInvalidGroups();

        // Create a memory buffer with the proper size
        Uint32 size = file_format.calcElementLength(xfer, EET_ExplicitLength);
        buffer.resize(size);

        // Create buffer for DCMTK
        DcmOutputBufferStream* outputstream =
                new DcmOutputBufferStream(&buffer[0], size);

        // Fill the memory buffer with the meta-header and the dataset
        file_format.transferInit();
        condition = file_format.write(*outputstream, xfer,
                                      EET_ExplicitLength, NULL);
        file_format.transferEnd();

        delete outputstream;

        if (condition.bad())
        {
            return CONVERSION_ERROR;
        }

        // Add the transfer syntax to the BSON object
        DcmDataset transfer_syntax_only_dicom;
        transfer_syntax_only_dicom.putAndInsertString(
            DCM_TransferSyntaxUID, DcmXfer(xfer).getXferID());
        object = mongo::BSONObjBuilder()
            .appendElements(object)
            .appendElements(dataset_to_bson(&transfer_syntax_only_dicom))
            .obj();
    }

    mongo::BSONObjBuilder meta_data_builder;
    meta_data_builder.appendElements(object);

    std::string const sopinstanceuid =
            object["00080018"].Obj().getField("Value").Array()[0].String();

    auto const use_gridfs = (meta_data_builder.len()+buffer.size()>16777000);

    if(use_gridfs)
    {
        mongo::GridFS gridfs(
            db_information.connection, db_information.bulk_data);
        mongo::BSONObj gridfs_object = gridfs.storeFile(
            buffer.c_str(), buffer.size(), sopinstanceuid);
        if(!gridfs_object.isValid() || gridfs_object.isEmpty())
        {
            // Return an error
            return INSERTION_FAILED;
        }

        // Update the meta-data builder with the reference to GridFS
        meta_data_builder << "Content"
            << gridfs_object.getField("_id").OID().toString();
    }
    else
    {
        if(db_information.db_name == db_information.bulk_data)
        {
            meta_data_builder.appendBinData(
                "Content", buffer.size(), mongo::BinDataGeneral,
                buffer.c_str());
        }
        else
        {
            mongo::BSONObjBuilder bulk_data_builder;
            bulk_data_builder.genOID();
            bulk_data_builder << "SOPInstanceUID" << sopinstanceuid;
            bulk_data_builder.appendBinData(
                "Content", buffer.size(), mongo::BinDataGeneral, buffer.c_str());
            auto bulk_data_object = bulk_data_builder.obj();

            db_information.connection.insert(
                db_information.bulk_data+".datasets", bulk_data_object);

            auto const result = db_information.connection.getLastError(
                db_information.bulk_data);
            if(result != "")
            {
                return INSERTION_FAILED;
            }

            // Update the meta-data builder with the reference to bulk-data
            meta_data_builder << "Content"
                << bulk_data_object.getField("_id").OID().toString();
        }
    }

    db_information.connection.insert(
        db_information.db_name+".datasets", meta_data_builder.obj());
    auto const result = db_information.connection.getLastError(
        db_information.db_name);
    if(!result.empty())
    {
        if(use_gridfs)
        {
            mongo::GridFS gridfs(
                db_information.connection, db_information.bulk_data);
            gridfs.removeFile(sopinstanceuid);
        }
        else if(db_information.db_name != db_information.bulk_data)
        {

            db_information.connection.remove(
                db_information.bulk_data+".datasets",
                BSON("SOPInstanceUID" << sopinstanceuid));
        }
        // Else db_information.db_name == db_information.bulk_data:
        // nothing to roll back

        return INSERTION_FAILED;
    }

    return NO_ERROR;
}

bool
is_dataset_allowed_for_storage(DataBaseInformation & db_information,
                               std::string const & username,
                               mongo::BSONObj const & dataset)
{
    // Retrieve user's Rights
    mongo::BSONObj constraint = get_constraint_for_user(
        db_information, username, Service_Store);

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
                    auto array = dataset[name].Obj().getField("Value").Array();
                    for(auto itarray = array.begin();
                        itarray != array.end(); ++itarray)
                    {
                        mongo::BSONElement const element2 = *itarray;
                        std::string valuestr = bsonelement_to_string(element2);

                        if (element.type() == mongo::BSONType::RegEx)
                        {
                            std::string regex(element.regex());
                            if (!boost::regex_match(valuestr.c_str(),
                                                    boost::regex(regex.c_str())))
                            {
                                result = false;
                                break;
                            }
                        }
                        else
                        {
                            std::string const elementstr =
                                    bsonelement_to_string(element);
                            if (valuestr != elementstr)
                            {
                                result = false;
                                break;
                            }
                        }
                    }
                    if (result == false)
                    {
                        break;
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

std::string
get_dataset_as_string(DataBaseInformation & db_information,
                      mongo::BSONObj object)
{
    mongo::BSONElement const content = object.getField("Content");
    if (content.type() == mongo::BSONType::String)
    {
        auto const sop_instance_uid =
            object["00080018"].Obj().getField("Value").Array()[0].String();

        auto const reference = content.String();
        mongo::GridFS gridfs(
            db_information.connection, db_information.bulk_data);
        auto const file = gridfs.findFile(sop_instance_uid);
        if(file.exists())
        {
            std::stringstream stream;
            file.write(stream);
            return stream.str();
        }
        else
        {
            auto const bulk_data = db_information.connection.findOne(
                db_information.bulk_data+".datasets",
                BSON("SOPInstanceUID" << sop_instance_uid));
            if(bulk_data.isEmpty())
            {
                throw ExceptionPACS("No bulk data");
            }
            auto const bulk_data_content = bulk_data.getField("Content");
            int size=0;
            char const * begin = bulk_data_content.binDataClean(size);

            return std::string(begin, size);
        }
    }
    else if (content.type() == mongo::BSONType::BinData)
    {
        int size=0;
        char const * begin = content.binDataClean(size);

        return std::string(begin, size);
    }
    else
    {
        std::stringstream streamerror;
        streamerror << "Unknown type '" << content.type()
                    << "' for Content attribute";
        throw ExceptionPACS(streamerror.str());
    }

    return "";
}

} // namespace services

} // namespace dopamine
