/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include "core/ConfigurationPACS.h"
#include "services/RetrieveGenerator.h"
#include "services/ServicesTools.h"
#include "Wado_rs.h"
#include "WebServiceException.h"

namespace dopamine
{

namespace services
{

Wado_rs
::Wado_rs(std::string const & pathinfo, std::string const & remoteuser):
    Wado(pathinfo, "", remoteuser)
{
    mongo::BSONObj const object = this->_parse_string();

    RetrieveGenerator generator(this->_username);

    Uint16 status = generator.process_bson(object);
    if (status != STATUS_Pending)
    {
        if ( ! generator.is_allow())
        {
            throw WebServiceException(401, "Authorization Required",
                                      authentication_string);
        }

        throw WebServiceException(500, "Internal Server Error",
                                  "Error while searching into database");
    }

    // Multipart Response
    this->_create_boundary();

    mongo::BSONObj findedobject = generator.next();
    if (!findedobject.isValid() || findedobject.isEmpty())
    {
        throw WebServiceException(404, "Not Found", "No Dataset");
    }

    std::stringstream stream;
    while (findedobject.isValid() && !findedobject.isEmpty())
    {
        std::string currentdata;
        try
        {
            currentdata = generator.retrieve_dataset_as_string(findedobject);
        }
        catch (ExceptionPACS exc)
        {
            throw WebServiceException(500, "Internal Server Error", exc.what());
        }

        std::string const filename = findedobject.getField("00080018").Obj().getField("Value").Array()[0].String();

        stream << "--" << this->_boundary << "\n";
        stream << CONTENT_TYPE << MIME_TYPE_APPLICATION_DICOM << "\n";
        stream << CONTENT_DISPOSITION_ATTACHMENT << " "
               << ATTRIBUT_FILENAME << filename << "\n";
        stream << CONTENT_TRANSFER_ENCODING << TRANSFER_ENCODING_BINARY << "\n" << "\n";

        stream << currentdata << "\n" << "\n";

        findedobject = generator.next();
    }

    // Close the response
    stream << "--" << this->_boundary << "--";

    this->_response = stream.str();
}

Wado_rs
::~Wado_rs()
{
    // Nothing to do
}

mongo::BSONObj
Wado_rs
::_parse_string()
{
    std::string study_instance_uid;
    std::string series_instance_uid;
    std::string sop_instance_uid;

    // Parse the path info
    // WARNING: inadequate method (TODO: find other method)
    // PATH_INFO is like: key1/value1/key2/value2
    std::vector<std::string> vartemp;
    boost::split(vartemp, this->_pathinfo, boost::is_any_of("/"),
                 boost::token_compress_off);

    if (vartemp.size() > 0 && vartemp[0] == "")
    {
        vartemp.erase(vartemp.begin());
    }

    if (vartemp.size() < 1)
    {
        throw WebServiceException(400, "Bad Request",
                                  "Some parameters missing");
    }

    // look for Study Instance UID
    if (vartemp[0] == "studies")
    {
        if (vartemp.size() < 2 || vartemp[1] == "")
        {
            throw WebServiceException(400, "Bad Request",
                                      "Missing study instance uid");
        }

        study_instance_uid = vartemp[1];

        // look for Series Instance UID
        if (vartemp.size() > 2)
        {
            if (vartemp[2] != "series")
            {
                throw WebServiceException(400, "Bad Request",
                                          "second parameter should be series");
            }

            if (vartemp.size() < 4 || vartemp[3] == "")
            {
                throw WebServiceException(400, "Bad Request",
                                          "Missing series instance uid");
            }

            series_instance_uid = vartemp[3];

            // look for SOP Instance UID
            if (vartemp.size() > 4)
            {
                if (vartemp[4] != "instances")
                {
                    throw WebServiceException(400, "Bad Request",
                                              "third parameter should be instances");
                }

                if (vartemp.size() < 6 || vartemp[5] == "")
                {
                    throw WebServiceException(400, "Bad Request",
                                              "Missing SOP instance uid");
                }

                sop_instance_uid = vartemp[5];
            }
        }
    }
    else
    {
        throw WebServiceException(400, "Bad Request",
                                  "first parameter should be studies");
    }

    // Request is valid

    // Conditions
    mongo::BSONObjBuilder db_query;

    if (sop_instance_uid != "")
    {
        db_query << "00080018" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(sop_instance_uid));
    }

    if (study_instance_uid != "")
    {
        db_query << "0020000d" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(study_instance_uid));
    }

    if (series_instance_uid != "")
    {
        db_query << "0020000e" << BSON("vr" << "UI" << "Value" << BSON_ARRAY(series_instance_uid));
    }

    std::string query_retrieve_level = sop_instance_uid != "" ? "IMAGE" :
                                       series_instance_uid != "" ? "SERIES" : "STUDY";
    db_query << "00080052" << BSON("vr" << "CS" << "Value" << BSON_ARRAY(query_retrieve_level));

    return db_query.obj();
}

} // namespace services

} // namespace dopamine
