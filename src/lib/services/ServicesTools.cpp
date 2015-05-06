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

#include "ConverterBSON/Dataset/BSONToDataSet.h"
#include "ConverterBSON/Dataset/DataSetToBSON.h"
#include "ConverterBSON/Dataset/IsPrivateTag.h"
#include "ConverterBSON/Dataset/VRMatch.h"
#include "core/ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "ServicesTools.h"

namespace dopamine
{

namespace services
{

bool
create_db_connection(mongo::DBClientConnection &connection, std::string &db_name)
{
    // Get all indexes
    std::string indexlist = ConfigurationPACS::get_instance().GetValue("database.indexlist");
    std::vector<std::string> indexeslist;
    boost::split(indexeslist, indexlist, boost::is_any_of(";"));

    db_name = ConfigurationPACS::get_instance().GetValue("database.dbname");
    std::string db_host = ConfigurationPACS::get_instance().GetValue("database.hostname");
    std::string db_port = ConfigurationPACS::get_instance().GetValue("database.port");

    if (db_name == "" || db_host == "" || db_port == "")
    {
        return false;
    }

    // Try to connect database
    // Disconnect is automatic when it calls the destructors
    std::string errormsg = "";
    if ( ! connection.connect(mongo::HostAndPort(db_host + ":" + db_port),
                                     errormsg) )
    {
        return false;
    }

    // Create indexes
    std::string const datasets = db_name + ".datasets";

    for (auto currentIndex : indexeslist)
    {
        DcmTag dcmtag;
        OFCondition ret = DcmTag::findTagFromName(currentIndex.c_str(), dcmtag);

        if (ret.good())
        {
            // convert Uint16 => string XXXXYYYY
            char buffer[9];
            snprintf(buffer, 9, "%04x%04x", dcmtag.getGroup(), dcmtag.getElement());

            std::stringstream stream;
            stream << "\"" << buffer << "\"";

            connection.ensureIndex
                (
                    datasets,
                    BSON(stream.str() << 1),
                    false,
                    std::string(dcmtag.getTagName())
                );
        }
    }

    return true;
}

void
createStatusDetail(const Uint16 &errorCode, const DcmTagKey &key,
                   const OFString &comment, DcmDataset **statusDetail)
{
    DcmElement * element;
    std::vector<Uint16> vect;
    OFCondition cond;

    (*statusDetail) = new DcmDataset();

    cond = (*statusDetail)->insertEmptyElement(DCM_Status);
    cond = (*statusDetail)->findAndGetElement(DCM_Status, element);
    vect.clear();
    vect.push_back(errorCode);
    cond = element->putUint16Array(&vect[0], vect.size());

    cond = (*statusDetail)->insertEmptyElement(DCM_OffendingElement);
    cond = (*statusDetail)->findAndGetElement(DCM_OffendingElement, element);
    vect.clear();
    vect.push_back(key.getGroup());
    vect.push_back(key.getElement());
    cond = element->putUint16Array(&vect[0], vect.size()/2);

    cond = (*statusDetail)->putAndInsertOFStringArray(DCM_ErrorComment,
                                                      comment);
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
        char * user; Uint16 user_length;
        userIdentNeg->getPrimField(user, user_length);
        // user is not NULL-terminated
        lcurrentUser = std::string(user, user_length);
        delete [] user;
    }

    return lcurrentUser;
}

bool
is_authorized(mongo::DBClientConnection &connection,
              std::string const & db_name,
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
                   << "service" << BSON("$in" << BSON_ARRAY(Service_All << servicename));

    mongo::BSONObj group_command = BSON("count" << "authorization" << "query" << fields_builder.obj());

    mongo::BSONObj info;
    connection.runCommand(db_name, group_command, info, 0);

    // If the command correctly executed and database entries match
    if (info["ok"].Double() == 1 && info["n"].Double() > 0)
    {
        return true;
    }

    // Not allowed
    return false;
}

mongo::BSONObj
get_constraint_for_user(mongo::DBClientConnection &connection, const std::string &db_name,
                        const std::string &username, const std::string &servicename)
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
                   << "service" << BSON("$in" << BSON_ARRAY(Service_All << servicename));
    mongo::BSONObjBuilder initial_builder;
    mongo::BSONObj group_command = BSON("group" << BSON(
        "ns" << "authorization" << "key" << BSON("dataset" << 1) << "cond" << fields_builder.obj() <<
        "$reduce" << "function(current, result) { }" << "initial" << initial_builder.obj()
    ));

    mongo::BSONObj result;
    connection.runCommand(db_name, group_command, result, 0);

    if (result["ok"].Double() != 1 || result["count"].Double() == 0)
    {
        // Create a constraint to deny all actions
        mongo::BSONObjBuilder builder_none;
        builder_none << "00080018.Value" << BSON_ARRAY("_db8eeea6_e0eb_48b8_9a02_a94926b76992");

        return builder_none.obj();
    }

    mongo::BSONArrayBuilder constaintarray;
    for (auto item : result["retval"].Array())
    {
        // if dataset: "" or dataset: {} => all is allowed
        if ((item["dataset"].type() == mongo::BSONType::String && item["dataset"].String() == "") ||
            (item["dataset"].type() == mongo::BSONType::Object && item["dataset"].Obj().isEmpty()))
        {
            // No constraint
            return mongo::BSONObj();
        }

        // Warning: throw an exception if item["dataset"].type() != mongo::BSONType::Object

        mongo::BSONArrayBuilder andarray;
        for(mongo::BSONObj::iterator it=item["dataset"].Obj().begin(); it.more();)
        {
            mongo::BSONElement const element = it.next();

            mongo::BSONObjBuilder object;
            if (element.type() == mongo::BSONType::RegEx)
            {
                object.appendRegex(std::string(element.fieldName())+".Value", element.regex(), "");
            }
            else
            {
                object << std::string(element.fieldName())+".Value" << bsonelement_to_string(element);
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
replace(const std::string &value, const std::string &old,
        const std::string &new_)
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
            converterBSON::IsPrivateTag::New(), converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            converterBSON::VRMatch::New(EVR_OB), converterBSON:: DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            converterBSON::VRMatch::New(EVR_OF), converterBSON:: DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            converterBSON::VRMatch::New(EVR_OW), converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(std::make_pair(
            converterBSON::VRMatch::New(EVR_UN), converterBSON::DataSetToBSON::FilterAction::EXCLUDE));
    }
    dataset_to_bson.set_default_filter(converterBSON::DataSetToBSON::FilterAction::INCLUDE);

    return dataset_to_bson.from_dataset(dataset);
}

DcmDataset *
bson_to_dataset(mongo::BSONObj object)
{
    DcmDataset* dataset = NULL;

    if ( ! object.hasField("location"))
    {
        converterBSON::BSONToDataSet bson2dataset;
        DcmDataset result = bson2dataset.to_dataset(object);
        dataset = new DcmDataset(result);
    }
    else
    {
        std::string const path = object.getField("location").String();
        DcmFileFormat fileformat;
        OFCondition result = fileformat.loadFile(path.c_str());
        if (result.bad())
        {
            loggerError() << "Cannot load dataset '" << path << "': "
                          << result.text();
            return NULL;
        }
        dataset = fileformat.getAndRemoveDataset();
    }

    return dataset;
}

} // namespace services

} // namespace dopamine
