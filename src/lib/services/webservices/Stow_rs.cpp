/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include <odil/DataSet.h>
#include <odil/Reader.h>
#include <odil/message/CStoreResponse.h>
#include <odil/registry.h>

#include <mimetic/mimeentity.h>

#include "ConverterBSON/bson_converter.h"
#include "core/dataset_tools.h"
#include "services/StoreGenerator.h"
#include "services/webservices/WebServiceException.h"
#include "Stow_rs.h"

namespace dopamine
{

namespace services
{

Stow_rs
::Stow_rs(std::string const & pathinfo,
          std::string const & querystring,
          std::string const & postdata,
          std::string const & remoteuser):
    Webservices(pathinfo, querystring),
    _content_type(""), _status(200), _code("OK")
{
    // Decode entries
    mongo::BSONObj object = this->_parse_string();

    // Multipart Response
    this->_create_boundary();

    // Parse MIME data and store dataset
    this->_process(postdata, object);
}

Stow_rs
::~Stow_rs()
{
    // Nothing to do
}

std::string
Stow_rs
::get_content_type() const
{
    return this->_content_type;
}

unsigned int
Stow_rs
::get_status() const
{
    return this->_status;
}

std::string
Stow_rs
::get_code() const
{
    return this->_code;
}

mongo::BSONObj
Stow_rs
::_parse_string()
{
    // Parse the path info
    // WARNING: inadequate method (TODO: find other method)
    // PATH_INFO is like: key1/value1/key2
    std::vector<std::string> vartemp;
    boost::split(vartemp, this->_pathinfo, boost::is_any_of("/"),
                 boost::token_compress_off);

    if (!vartemp.empty() && vartemp[0] == "")
    {
        vartemp.erase(vartemp.begin());
    }

    if (vartemp.size() < 1)
    {
        throw WebServiceException(400, "Bad Request",
                                  "Some parameters missing");
    }
    if (vartemp.size() > 2)
    {
        throw WebServiceException(400, "Bad Request",
                                  "Too many parameters");
    }

    std::string study_instance_uid;

    // look for Study Instance UID
    if (vartemp[0] == "studies")
    {
        if (vartemp.size() > 1)
        {
            study_instance_uid = vartemp[1];
        }
    }
    else
    {
        throw WebServiceException(400, "Bad Request",
                                  "first parameter should be studies");
    }

    // Conditions
    mongo::BSONObjBuilder db_query;

    if (study_instance_uid != "")
    {
        db_query << "0020000d"
                 << BSON("vr" << "UI" <<
                         "Value" << BSON_ARRAY(study_instance_uid));
    }

    return db_query.obj();
}

std::string
Stow_rs
::_find_content_type(std::string const & contenttype)
{
    if (contenttype.find(MIME_TYPE_APPLICATION_DICOMXML) != std::string::npos)
    {// search this before MIME_TYPE_APPLICATION_DICOM
        std::stringstream streamerror;
        streamerror << "This media type " << MIME_TYPE_APPLICATION_DICOMXML
                    << " is not yet supported by the server.";
        throw WebServiceException(415, "Unsupported Media Type",
                                  streamerror.str());
        //return MIME_TYPE_APPLICATION_DICOMXML;
    }
    else if (contenttype.find(MIME_TYPE_APPLICATION_DICOM) != std::string::npos)
    {
        return MIME_TYPE_APPLICATION_DICOM;
    }
    else if (contenttype.find(MIME_TYPE_APPLICATION_JSON) != std::string::npos)
    {
        std::stringstream streamerror;
        streamerror << "This media type " << MIME_TYPE_APPLICATION_JSON
                    << " is not yet supported by the server.";
        throw WebServiceException(415, "Unsupported Media Type",
                                  streamerror.str());
        //return MIME_TYPE_APPLICATION_JSON;
    }

    throw WebServiceException(400, "Bad Request",
                              "Unknown Media type");
}

void
Stow_rs
::_process(std::string const & postdata, mongo::BSONObj const & studyinstanceuid)
{
    // Parse MIME Message
    std::stringstream stream;
    stream << postdata;
    mimetic::MimeEntity entity(stream);

    // Check header
    mimetic::Header& h = entity.header();

    if (h.contentType().isMultipart() == false)
    {
        throw WebServiceException(400, "Bad Request",
                                  "Content-type should be multipart/related");
    }
    this->_content_type = this->_find_content_type(h.contentType().str());

    odil::DataSet responseDataset;
    try
    {
        responseDataset.add(odil::registry::RetrieveURL, odil::VR::UT);
        responseDataset.add(odil::registry::FailedSOPSequence,
                            odil::VR::SQ);
        responseDataset.add(odil::registry::ReferencedSOPSequence,
                            odil::VR::SQ);

        mimetic::MimeEntityList& parts = entity.body().parts();
        // cycle on sub entities list and print info of every item
        for(mimetic::MimeEntityList::iterator mbit = parts.begin();
            mbit != parts.end(); ++mbit)
        {
            // check header
            mimetic::Header& header = (*mbit)->header();
            std::stringstream contenttypestream;
            contenttypestream << header.contentType();
            if (this->_content_type != contenttypestream.str())
            {
                // ERROR: add an item into failedsopsequence
                odil::DataSet failedsopsequence;
                failedsopsequence.add(odil::registry::FailureReason,
                                      odil::Element({0x0110},
                                                       odil::VR::US));
                failedsopsequence.add(odil::registry::ReferencedSOPClassUID,
                                      odil::Element({"Unknown"},
                                                       odil::VR::UI));
                failedsopsequence.add(
                            odil::registry::ReferencedSOPInstanceUID,
                            odil::Element({"Unknown"}, odil::VR::UI));
                responseDataset.as_data_set(
                            odil::registry::FailedSOPSequence).push_back(
                                    failedsopsequence);
                continue;
            }

            mimetic::Body& body = (*mbit)->body();

            odil::DataSet dataset;

            // see PS3.18 6.6.1.1.1 DICOM Request Message Body
            if (this->_content_type == MIME_TYPE_APPLICATION_DICOM)
            {
                // remove the ended boundary
                std::string temp(body.c_str(), body.size());
                temp = temp.substr(0, temp.rfind("\n\n--"));

                // remove ended '\n'
                while (temp[temp.size()-1] == '\n')
                {
                    temp = temp.substr(0, temp.rfind("\n"));
                }

                try
                {
                    std::stringstream stream; stream << temp;
                    auto file = odil::Reader::read_file(stream);
                    dataset = file.second;
                }
                catch (std::exception const & exc)
                {
                    // ERROR: add an item into failedsopsequence
                    odil::DataSet failedsopsequence;
                    failedsopsequence.add(odil::registry::FailureReason,
                                          odil::Element({0xa700},
                                                           odil::VR::US));
                    failedsopsequence.add(
                                odil::registry::ReferencedSOPClassUID,
                                odil::Element({"Unknown"}, odil::VR::UI));
                    failedsopsequence.add(
                                odil::registry::ReferencedSOPInstanceUID,
                                odil::Element({"Unknown"}, odil::VR::UI));
                    responseDataset.as_data_set(
                                odil::registry::FailedSOPSequence).push_back(
                                        failedsopsequence);
                    continue;
                }
            }
            else
            {
                std::stringstream streamerror;
                streamerror << "Content-type for each part should be "
                            << MIME_TYPE_APPLICATION_DICOM;
                throw WebServiceException(400, "Bad Request",
                                          streamerror.str());
            }

            auto const sopclassuid =
                    dataset.as_string(odil::registry::SOPClassUID)[0];
            auto const sopinstanceuid =
                    dataset.as_string(odil::registry::SOPInstanceUID)[0];

            // Modify dataset here (see PS3.18 6.6.1.2 Action)

            uint16_t result = odil::message::Response::Pending;
            // Check StudyInstanceUID
            if (!studyinstanceuid.isEmpty())
            {
                auto const studyuid = dataset.as_string(
                            odil::registry::StudyInstanceUID)[0];
                mongo::BSONObj const studyobj =
                        studyinstanceuid["0020000d"].Obj();
                if (studyobj["Value"].Array()[0].String() !=
                    std::string(studyuid.c_str()))
                {
                    result = 0xa700;
                }
            }

            if (result == odil::message::Response::Pending)
            {
                // Insert dataset into DataBase
                StoreGenerator::Pointer generator = StoreGenerator::New();
                result = generator->initialize(dataset);

                if (result ==
                    odil::message::CStoreResponse::RefusedNotAuthorized)
                {
                    throw WebServiceException(401, "Unauthorized",
                                              authentication_string);
                }
            }

            if (result != odil::message::Response::Success)
            {
                // ERROR: add an item into failedsopsequence
                odil::DataSet failedsopsequence;
                failedsopsequence.add(odil::registry::FailureReason,
                                      odil::Element({result},
                                                       odil::VR::US));
                failedsopsequence.add(odil::registry::ReferencedSOPClassUID,
                                      odil::Element({sopclassuid},
                                                       odil::VR::UI));
                failedsopsequence.add(
                            odil::registry::ReferencedSOPInstanceUID,
                            odil::Element({sopinstanceuid}, odil::VR::UI));
                responseDataset.as_data_set(
                            odil::registry::FailedSOPSequence).push_back(
                                    failedsopsequence);
            }
            else
            {
                // Everything is OK
                odil::DataSet referencedsopsequence;
                referencedsopsequence.add(odil::registry::RetrieveURL,
                                          odil::VR::UT);
                referencedsopsequence.add(
                            odil::registry::ReferencedSOPClassUID,
                            odil::Element({sopclassuid}, odil::VR::UI));
                referencedsopsequence.add(
                            odil::registry::ReferencedSOPInstanceUID,
                            odil::Element({sopinstanceuid}, odil::VR::UI));
                responseDataset.as_data_set(
                            odil::registry::ReferencedSOPSequence).push_back(
                                    referencedsopsequence);
            }
        }

        bool containsbad =
                responseDataset.as_data_set(
                    odil::registry::FailedSOPSequence).size() != 0;
        // Check sequences
        if (!containsbad)
        {
            // empty sequence => remove
            responseDataset.remove(odil::registry::FailedSOPSequence);
        }
        bool containsgood =
                responseDataset.as_data_set(
                    odil::registry::ReferencedSOPSequence).size() != 0;
        if (!containsgood)
        {
            // empty sequence => remove
            responseDataset.remove(odil::registry::ReferencedSOPSequence);
        }

        // See PS3.18 Table 6.6.1-1. HTTP/1.1 Standard Response Code
        if (containsbad && !containsgood)
        {
            this->_status = 409;
            this->_code = "Conflict";
        }
        else if (containsbad && containsgood)
        {
            this->_status = 202;
            this->_code = "Accepted";
        }
        else
        {
            this->_status = 200;
            this->_code = "OK";
        }

        // Transfert DcmDataset into XML
        this->_response = dataset_to_xml_string(responseDataset);
    }
    catch (odil::Exception const & dcmtkppexc)
    {
        throw WebServiceException(503, "Busy", std::string(dcmtkppexc.what()));
    }
}

} // namespace services

} // namespace dopamine
