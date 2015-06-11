/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include "boost/regex.hpp"

/* make sure OS specific configuration is included first */
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctag.h>

#include "ConverterBSON/JSON/BSONToJSON.h"
#include "ConverterBSON/XML/BSONToXML.h"
#include "Qido_rs.h"
#include "services/QueryGenerator.h"
#include "services/ServicesTools.h"
#include "WebServiceException.h"

namespace dopamine
{

namespace services
{

mongo::BSONObj
check_mandatory_field_in_response(mongo::BSONObj const & response,
                                  QueryGenerator &generator,
                                  std::string const & query_retrieve_level,
                                  std::vector<Attribute> attributes)
{
    mongo::BSONObjBuilder builderfinal;
    builderfinal.appendElements(response);

    // Add Query retrieve level
    mongo::BSONObj queryretrievelevel =
            BSON("00080052" <<
                 BSON("vr" << "CS" <<
                      "Value" << BSON_ARRAY(query_retrieve_level)));
    builderfinal.appendElements(queryretrievelevel);

    for (Attribute attribute : attributes)
    {
        if (attribute.get_tag() == "00080052") continue;
        if (!response.hasField(attribute.get_tag()))
        {
            // Instance Availability
            if (attribute.get_tag() == "00080056")
            {
                builderfinal.appendElements(
                            generator.compute_attribute("00080056", ""));
            }
            // Modalities in Study
            else if (attribute.get_tag() == "00080061")
            {
                std::string const value =
                        response["0020000d"].Obj()["Value"].Array()[0].String();
                builderfinal.appendElements(
                            generator.compute_attribute("00080061", value));
            }
            // Number of Study Related Series
            else if (attribute.get_tag() == "00201206")
            {
                std::string const value =
                        response["0020000d"].Obj()["Value"].Array()[0].String();
                builderfinal.appendElements(
                            generator.compute_attribute("00201206", value));
            }
            // Number of Study Related Instances
            else if (attribute.get_tag() == "00201208")
            {
                std::string const value =
                        response["0020000d"].Obj()["Value"].Array()[0].String();
                builderfinal.appendElements(
                            generator.compute_attribute("00201208", value));
            }
            // Number of Series Related Instances
            else if (attribute.get_tag() == "00201209")
            {
                std::string const value =
                        response["0020000e"].Obj()["Value"].Array()[0].String();
                builderfinal.appendElements(
                            generator.compute_attribute("00201209", value));
            }
            else
            {
                mongo::BSONObjBuilder builder;
                builder << "vr" << attribute.get_vr();
                builder.appendNull("Value");

                mongo::BSONObj element =
                        BSON(attribute.get_tag() << builder.obj());

                builderfinal.appendElements(element);
            }
        }
    }

    return builderfinal.obj();
}

Qido_rs
::Qido_rs(std::string const & pathinfo,
          std::string const & querystring,
          std::string const & contenttype,
          std::string const & remoteuser):
    Webservices(pathinfo, querystring, remoteuser),
    _contenttype(contenttype), _study_instance_uid_present(false),
    _series_instance_uid_present(false)
{
    mongo::BSONObj object = this->_parse_string();

    this->_add_mandatory_fields(object);

    QueryGenerator generator(this->_username);
    generator.set_include_fields(this->_includefields);
    generator.set_maximum_results(this->_maximum_results);
    generator.set_skipped_results(this->_skipped_results);
    generator.set_fuzzy_matching(this->_fuzzy_matching);

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

    std::stringstream stream;

    mongo::BSONObj findedobject = generator.next();
    if (!findedobject.isValid() || findedobject.isEmpty())
    {
        // If there are no matching results, the message body will be empty.
        if (this->_contenttype == MIME_TYPE_APPLICATION_DICOMXML)
        {
            stream << "\n\n";
        }
        // If there are no matching results, the JSON message is empty.
        else if (this->_contenttype == MIME_TYPE_APPLICATION_JSON)
        {
            stream << "[\n";
        }
        else
        {
            throw WebServiceException(404, "Not Found", "No Dataset");
        }
    }

    bool isfirst = true;
    while (findedobject.isValid() && !findedobject.isEmpty())
    {
        findedobject = check_mandatory_field_in_response(
                            findedobject, generator,
                            this->_query_retrieve_level,
                            this->_get_mandatory_fields());

        if (this->_contenttype == MIME_TYPE_APPLICATION_JSON)
        {
            if (isfirst)
            {
                stream << "[\n";
                isfirst = false;
            }
            else
            {
                stream << ",\n";
            }
            converterBSON::BSONToJSON bsontojson;
            stream << bsontojson.to_string(findedobject);
        }
        else if (this->_contenttype == MIME_TYPE_APPLICATION_DICOMXML)
        {
            stream << "--" << this->_boundary << "\n";
            stream << CONTENT_TYPE << MIME_TYPE_APPLICATION_DICOMXML << "\n\n";

            converterBSON::BSONToXML bsontoxml;
            std::string currentdata = bsontoxml.to_string(findedobject);

            // The directive xml:space="preserve" shall be included.
            std::stringstream xmlheader;
            xmlheader << "<?xml version=\"1.0\" "
                      << "encoding=\"utf-8\" xml:space=\"preserve\" ?>";
            currentdata = replace(
                            currentdata,
                            "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
                            xmlheader.str());

            stream << currentdata << "\n\n";
        }

        findedobject = generator.next();
    }

    if (this->_contenttype == MIME_TYPE_APPLICATION_JSON)
    {
        stream << "]\n";
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
                                    throw WebServiceException(
                                                400, "Bad Request",
                                                "too many parameters");
                                }
                            }
                            else
                            {
                                throw WebServiceException(
                                        400, "Bad Request",
                                        "third parameter should be instances");
                            }
                        }
                        else
                        {
                            throw WebServiceException(
                                        400, "Bad Request",
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
                    throw WebServiceException(
                            400, "Bad Request",
                            "second parameter should be series or instances");
                }
            }
            else
            {
                throw WebServiceException(
                            400, "Bad Request",
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
        throw WebServiceException(
                    400, "Bad Request",
                    "first parameter should be studies, series or instances");
    }

    // Conditions
    mongo::BSONObjBuilder db_query;

    this->_study_instance_uid_present = study_instance_uid != "";
    if (study_instance_uid != "")
    {
        db_query << "0020000d"
                 << BSON("vr" << "UI" <<
                         "Value" << BSON_ARRAY(study_instance_uid));
    }

    this->_series_instance_uid_present = series_instance_uid != "";
    if (series_instance_uid != "")
    {
        db_query << "0020000e"
                 << BSON("vr" << "UI" <<
                         "Value" << BSON_ARRAY(series_instance_uid));
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
            this->_add_to_builder(tempbuilder, data[1], "");
            this->_includefields.push_back(
                        tempbuilder.obj().firstElementFieldName());
        }
        else if (tag == "limit")
        {
            this->_maximum_results = std::atoi(data[1].c_str());
        }
        else if (tag == "offset")
        {
            this->_skipped_results = std::atoi(data[1].c_str());
            if (this->_skipped_results < 0)
            {
                this->_skipped_results = 0;
            }
        }
        else if (tag == "fuzzymatching")
        {
            // Not supported
            this->_fuzzy_matching = (data[1] == "true");

            if (this->_fuzzy_matching)
            {
                /* 6.7.1.2.1 Matching
                 * If the "fuzzymatching=true" query key/value is
                 * included in the request and it is not supported,
                 * the response shall include the following
                 * HTTP/1.1 Warning header
                 */
                std::stringstream streamerror;
                streamerror << "The fuzzymatching parameter is not supported. "
                            << "Only literal matching has been performed.";
                throw WebServiceException(299, "Warning",
                                          streamerror.str());
            }
        }
        else
        {
            this->_add_to_builder(db_query, tag, data[1]);
        }
    }
    arraytemp.clear();

    if (!db_query.hasField("00080052"))
    {
        db_query << "00080052"
                 << BSON("vr" << "CS" <<
                         "Value" << BSON_ARRAY(this->_query_retrieve_level));
    }

    return db_query.obj();
}

void
Qido_rs
::_add_to_builder(mongo::BSONObjBuilder &builder,
                  std::string const & tag,
                  std::string const & value)
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

    if (response.getVR().getEVR() == EVR_PN)
    {
        builder << groupelement.str()
                << BSON("vr" << response.getVR().getVRName() <<
                        "Value" << BSON_ARRAY(BSON("Alphabetic" << value)));
    }
    else if (response.getVR().getEVR() == EVR_OB ||
             response.getVR().getEVR() == EVR_OF ||
             response.getVR().getEVR() == EVR_OW ||
             response.getVR().getEVR() == EVR_UN)
    {
        builder << groupelement.str()
                << BSON("vr" << response.getVR().getVRName() <<
                        "InlineBinary" << value);
    }
    else
    {
        builder << groupelement.str()
                << BSON("vr" << response.getVR().getVRName() <<
                        "Value" << BSON_ARRAY(value));
    }
}

std::vector<Attribute>
Qido_rs
::_get_mandatory_fields() const
{
    std::vector<Attribute> tag_to_add;
    if (this->_query_retrieve_level == "STUDY")
    { // See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
        tag_to_add.insert(tag_to_add.end(),
                          mandatory_study_attributes.begin(),
                          mandatory_study_attributes.end());
    }
    else if (this->_query_retrieve_level == "SERIES")
    { // See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
        if (!this->_study_instance_uid_present)
        {
            tag_to_add.insert(tag_to_add.end(),
                              mandatory_study_attributes.begin(),
                              mandatory_study_attributes.end());
        }
        tag_to_add.insert(tag_to_add.end(),
                          mandatory_series_attributes.begin(),
                          mandatory_series_attributes.end());
    }
    else
    { // See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
        if (!this->_study_instance_uid_present)
        {
            tag_to_add.insert(tag_to_add.end(),
                              mandatory_study_attributes.begin(),
                              mandatory_study_attributes.end());
        }
        if (!this->_series_instance_uid_present)
        {
            tag_to_add.insert(tag_to_add.end(),
                              mandatory_series_attributes.begin(),
                              mandatory_series_attributes.end());
        }
        tag_to_add.insert(tag_to_add.end(),
                          mandatory_instance_attributes.begin(),
                          mandatory_instance_attributes.end());
    }

    return tag_to_add;
}

void
Qido_rs
::_add_mandatory_fields(mongo::BSONObj const & queryobject)
{
    // Add tags
    for (Attribute tag : this->_get_mandatory_fields())
    {
        // tag not already added
        if (std::find(this->_includefields.begin(),
                      this->_includefields.end(),
                      tag.get_tag()) != this->_includefields.end() ||
            queryobject.hasField(tag.get_tag()))
        {
            continue;
        }

        this->_includefields.push_back(tag.get_tag());
    }
}

} // namespace services

} // namespace dopamine
