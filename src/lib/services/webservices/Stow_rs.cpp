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

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcistrmb.h>

#include <mimetic/mimeentity.h>

#include "ConverterBSON/Dataset/DataSetToBSON.h"
#include "ConverterBSON/XML/BSONToXML.h"
#include "services/ServicesTools.h"
#include "services/StoreGenerator.h"
#include "Stow_rs.h"
#include "WebServiceException.h"

#define DCM_RetrieveURL DcmTagKey(0x0008, 0x1190)

namespace dopamine
{

namespace services
{

Stow_rs
::Stow_rs(std::string const & pathinfo,
          std::string const & querystring,
          std::string const & postdata,
          std::string const & remoteuser):
    Webservices(pathinfo, querystring, remoteuser),
    _content_type(""), _status(200), _code("OK")
{
    // First add unreferenced DICOM Tag
    DcmDataDictionary &dictionary = dcmDataDict.wrlock();
    dictionary.addEntry(new DcmDictEntry(0x0008, 0x1190, DcmVR("UT"), "RetrieveURL",
                                         1, 1, NULL, OFTrue, NULL));
    dcmDataDict.unlock();

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

    if (vartemp.size() > 0 && vartemp[0] == "")
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
        db_query << "0020000d" << BSON("vr" << "UI" <<
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

    DcmDataset responseDataset;
    OFCondition condition = responseDataset.insertEmptyElement(DCM_RetrieveURL, true);
    if (condition.bad())
    {
        throw WebServiceException(503, "Busy", std::string(condition.text()));
    }

    DcmSequenceOfItems* failedsopsequence = NULL;
    condition = responseDataset.insertEmptyElement(DCM_FailedSOPSequence);
    if (condition.bad())
    {
        throw WebServiceException(503, "Busy", std::string(condition.text()));
    }
    condition = responseDataset.findAndGetSequence(DCM_FailedSOPSequence, failedsopsequence);
    if (condition.bad())
    {
        throw WebServiceException(503, "Busy", std::string(condition.text()));
    }

    DcmSequenceOfItems* referencedsopsequence = NULL;
    condition = responseDataset.insertEmptyElement(DCM_ReferencedSOPSequence);
    if (condition.bad())
    {
        throw WebServiceException(503, "Busy", std::string(condition.text()));
    }
    condition = responseDataset.findAndGetSequence(DCM_ReferencedSOPSequence, referencedsopsequence);
    if (condition.bad())
    {
        throw WebServiceException(503, "Busy", std::string(condition.text()));
    }

    mimetic::MimeEntityList& parts = entity.body().parts(); // list of sub entities obj
    // cycle on sub entities list and print info of every item
    for(mimetic::MimeEntityList::iterator mbit = parts.begin();
        mbit != parts.end(); ++mbit)
    {
        DcmItem* item = new DcmItem();

        // check header
        mimetic::Header& header = (*mbit)->header();
        std::stringstream contenttypestream;
        contenttypestream << header.contentType();
        if (this->_content_type != contenttypestream.str())
        {
            // ERROR: add an item into failedsopsequence
            item->putAndInsertUint16(DCM_FailureReason, 0x0110, 0);
            item->putAndInsertOFStringArray(DCM_ReferencedSOPClassUID, OFString("Unknown"));
            item->putAndInsertOFStringArray(DCM_ReferencedSOPInstanceUID, OFString("Unknown"));

            failedsopsequence->append(item);
            continue;
        }

        mimetic::Body& body = (*mbit)->body();

        DcmDataset* dataset = NULL;

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

            // Create buffer for DCMTK
            DcmInputBufferStream* inputbufferstream = new DcmInputBufferStream();
            inputbufferstream->setBuffer(temp.c_str(), temp.size());
            inputbufferstream->setEos();

            // Convert buffer into Dataset
            DcmFileFormat fileformat;
            fileformat.transferInit();
            condition = fileformat.read(*inputbufferstream);
            fileformat.transferEnd();

            delete inputbufferstream;

            if (condition.bad())
            {
                // ERROR: add an item into failedsopsequence
                item->putAndInsertUint16(DCM_FailureReason, 0xa700, 0);
                item->putAndInsertOFStringArray(DCM_ReferencedSOPClassUID, OFString("Unknown"));
                item->putAndInsertOFStringArray(DCM_ReferencedSOPInstanceUID, OFString("Unknown"));

                failedsopsequence->append(item);
                continue;
            }
            dataset = fileformat.getAndRemoveDataset();
        }

        OFString sopclassuid;
        dataset->findAndGetOFStringArray(DCM_SOPClassUID, sopclassuid);
        OFString sopinstanceuid;
        dataset->findAndGetOFStringArray(DCM_SOPInstanceUID, sopinstanceuid);

        item->putAndInsertOFStringArray(DCM_ReferencedSOPClassUID, sopclassuid);
        item->putAndInsertOFStringArray(DCM_ReferencedSOPInstanceUID, sopinstanceuid);

        // Modify dataset here (see PS3.18 6.6.1.2 Action)

        Uint16 result = STATUS_Pending;
        // Check StudyInstanceUID
        if (!studyinstanceuid.isEmpty())
        {
            OFString studyuid;
            dataset->findAndGetOFStringArray(DCM_StudyInstanceUID, studyuid);
            if (studyinstanceuid["0020000d"].Obj()["Value"].Array()[0].String() != std::string(studyuid.c_str()))
            {
                result = 0xa700;
            }
        }

        if (result == STATUS_Pending)
        {
            // Insert dataset into DataBase
            StoreGenerator generator(this->_username);
            result = generator.process_dataset(dataset, true);

            if ( ! generator.is_allow())
            {
                throw WebServiceException(401, "Unauthorized",
                                          authentication_string);
            }
        }

        if (result != STATUS_Pending)
        {
            // ERROR: add an item into failedsopsequence
            item->putAndInsertUint16(DCM_FailureReason, result, 0);
            failedsopsequence->append(item);
        }
        else
        {
            // Everything is OK
            item->insertEmptyElement(DCM_RetrieveURL, true);
            referencedsopsequence->append(item);
        }

        if (dataset != NULL)
        {
            delete dataset;
        }
    }

    bool containsbad = failedsopsequence->getLength() != 0;
    // Check sequences
    if (failedsopsequence->getLength() == 0)
    {
        // empty sequence => remove
        responseDataset.findAndDeleteElement(DCM_FailedSOPSequence, true);
    }
    bool containsgood = referencedsopsequence->getLength() != 0;
    if (referencedsopsequence->getLength() == 0)
    {
        // empty sequence => remove
        responseDataset.findAndDeleteElement(DCM_ReferencedSOPSequence, true);
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
    converterBSON::DataSetToBSON datasettobson;
    datasettobson.set_default_filter(converterBSON::DataSetToBSON::FilterAction::INCLUDE);
    mongo::BSONObj bsondataset = datasettobson.from_dataset(&responseDataset);
    converterBSON::BSONToXML bsontoxml;
    this->_response = bsontoxml.to_string(bsondataset);
}

} // namespace services

} // namespace dopamine
