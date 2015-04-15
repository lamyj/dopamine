/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "boost/regex.hpp"

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dctag.h>

#include "ConverterBSON/XML/BSONToXML.h"
#include "Qido_rs.h"
#include "services/QueryGenerator.h"
#include "services/ServicesTools.h"
#include "WebServiceException.h"

namespace dopamine
{

namespace services
{

Qido_rs
::Qido_rs(const std::string &pathinfo,
          const std::string &querystring,
          const std::string &contenttype,
          const std::string &remoteuser):
    Webservices(pathinfo, querystring, remoteuser),
    _contenttype(contenttype)
{
    mongo::BSONObj object = this->parse_string();

    QueryGenerator generator(this->_username);
    generator.set_includefields(this->_includefields);

    Uint16 status = generator.set_query(object);
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
    this->create_boundary();

    mongo::BSONObj findedobject = generator.next();
    if (!findedobject.isValid() || findedobject.isEmpty())
    {
        throw WebServiceException(404, "Not Found", "No Dataset");
    }

    std::stringstream stream;
    mongo::BSONArrayBuilder bsonarraybuilder; // only for Content-Type = MIME_TYPE_APPLICATION_JSON
    while (findedobject.isValid() && !findedobject.isEmpty())
    {
        if (this->_contenttype == MIME_TYPE_APPLICATION_JSON)
        {
            bsonarraybuilder << findedobject;
        }
        else if (this->_contenttype == MIME_TYPE_APPLICATION_DICOMXML)
        {
            stream << "--" << this->_boundary << "\n";
            stream << CONTENT_TYPE << MIME_TYPE_APPLICATION_DICOMXML << "\n";
            //TODO RLA stream << CONTENT_DISPOSITION_ATTACHMENT << " "
            //TODO RLA        << ATTRIBUT_FILENAME << this->_filename << "\n";
            stream << CONTENT_TRANSFER_ENCODING << TRANSFER_ENCODING_BINARY << "\n" << "\n";

            dopamine::BSONToXML bsontoxml;
            boost::property_tree::ptree tree = bsontoxml(findedobject);

            std::stringstream xmldataset;
            boost::property_tree::xml_writer_settings<char> settings(' ', 4);
            boost::property_tree::write_xml(xmldataset, tree, settings);

            std::string currentdata = xmldataset.str();

            // The directive xml:space="preserve" shall be included.
            currentdata = replace(currentdata,
                                  "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
                                  "<?xml version=\"1.0\" encoding=\"utf-8\" xml:space=\"preserve\" ?>");

            stream << currentdata << "\n" << "\n";
        }

        findedobject = generator.next();
    }

    if (this->_contenttype == MIME_TYPE_APPLICATION_JSON)
    {
        stream << bsonarraybuilder.arr().toString();
    }
    else if (this->_contenttype == MIME_TYPE_APPLICATION_DICOMXML)
    {
        // Close the response
        stream << "--" << this->_boundary << "--";
    }

    this->_response = stream.str();
}

Qido_rs
::~Qido_rs()
{
    // Nothing to do
}

std::string
Qido_rs
::get_contenttype() const
{
    return this->_contenttype;
}

mongo::BSONObj
Qido_rs
::parse_string()
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

    std::string study_instance_uid;
    std::string series_instance_uid;

    // look for Study Instance UID
    if (vartemp[0] == "studies")
    {
        _query_retrieve_level = "STUDY";
        if (vartemp.size() > 1)
        {
            study_instance_uid = vartemp[1];

            if (vartemp.size() > 2)
            {
                if (vartemp[2] == "series")
                {
                    _query_retrieve_level = "SERIES";
                    if (vartemp.size() > 3)
                    {
                        series_instance_uid = vartemp[3];

                        if (vartemp.size() > 4)
                        {
                            if (vartemp[4] == "instances")
                            {
                                _query_retrieve_level = "IMAGE";

                                if (vartemp.size() > 5)
                                {

                                    throw WebServiceException(400, "Bad Request",
                                                              "too many parameters");
                                }
                            }
                            else
                            {
                                throw WebServiceException(400, "Bad Request",
                                                          "third parameter should be instances");
                            }
                        }
                        else
                        {
                            throw WebServiceException(400, "Bad Request",
                                                      "third parameter should be instances");
                        }
                    }
                }
                else if (vartemp[2] == "instances")
                {
                    _query_retrieve_level = "IMAGE";

                    if (vartemp.size() > 3)
                    {

                        throw WebServiceException(400, "Bad Request",
                                                  "too many parameters");
                    }
                }
                else
                {
                    throw WebServiceException(400, "Bad Request",
                                              "second parameter should be series or instances");
                }
            }
            else
            {
                throw WebServiceException(400, "Bad Request",
                                          "second parameter should be series or instances");
            }
        }
    }
    else if (vartemp[0] == "series")
    {
        _query_retrieve_level = "SERIES";

        if (vartemp.size() > 1)
        {

            throw WebServiceException(400, "Bad Request",
                                      "too many parameters");
        }
    }
    else if (vartemp[0] == "instances")
    {
        _query_retrieve_level = "IMAGE";

        if (vartemp.size() > 1)
        {
            throw WebServiceException(400, "Bad Request",
                                      "too many parameters");
        }
    }
    else
    {
        throw WebServiceException(400, "Bad Request",
                                  "first parameter should be studies, series or instances");
    }

    // Conditions
    mongo::BSONObjBuilder db_query;

    if (study_instance_uid != "")
    {
        db_query << "0020000d" << BSON_ARRAY("UI" << study_instance_uid);
    }

    if (series_instance_uid != "")
    {
        db_query << "0020000e" << BSON_ARRAY("UI" << series_instance_uid);
    }

    // Parse the query string
    // WARNING: inadequate method (TODO: find other method)
    // Query string is like: name1=value1&name2=value2
    std::vector<std::string> arraytemp;
    boost::split(arraytemp, this->_querystring, boost::is_any_of("&"));

    this->_includefields.clear();
    for (std::string variable : arraytemp)
    {
        std::vector<std::string> data;
        boost::split(data, variable, boost::is_any_of("="));

        std::string tag = data[0];
        if (tag == "includefield")
        {
            mongo::BSONObjBuilder tempbuilder;
            this->add_to_builder(tempbuilder, data[1], "");
            this->_includefields.push_back(tempbuilder.obj().firstElementFieldName());
        }
        else
        {
            this->add_to_builder(db_query, tag, data[1]);
        }
    }
    arraytemp.clear();

    if (!db_query.hasField("00080052"))
    {
        db_query << "00080052" << BSON_ARRAY("CS" << this->_query_retrieve_level);
    }

    return db_query.obj();
}

void
Qido_rs
::add_to_builder(mongo::BSONObjBuilder &builder,
                 const std::string &tag, const std::string &value)
{
    std::string tagstr = tag;

    const boost::regex RegEx_tag("^[0-9a-fA-F]{8}$");

    boost::cmatch what;
    if (regex_match(tag.c_str(), what, RegEx_tag))
    {// match, value is tag XXXXYYYY
        tagstr.insert(tagstr.begin() + 4, ',');
    }
    // else value is Keyword

    DcmTag response;
    OFCondition condition = DcmTag::findTagFromName(tagstr.c_str(), response);
    if (condition.bad())
    {
        std::stringstream stream;
        stream << "Unknown DICOM Tag: " << tag;
        throw WebServiceException(400, "Bad Request",
                                  stream.str());
    }

    std::stringstream streamgroup;
    streamgroup << std::hex << response.getGroup();
    std::string group = streamgroup.str();
    while (group.length() != 4)
    {
        group = "0" + group;
    }

    std::stringstream streamelement;
    streamelement << std::hex << response.getElement();
    std::string element = streamelement.str();
    while (element.length() != 4)
    {
        element = "0" + element;
    }
    std::stringstream groupelement;
    groupelement << group << element;

    builder << groupelement.str() << BSON_ARRAY(response.getVR().getVRName() << value);
}

} // namespace services

} // namespace dopamine
