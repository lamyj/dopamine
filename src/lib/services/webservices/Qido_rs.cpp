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

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dctag.h>

#include "ConverterBSON/JSON/BSONToJSON.h"
#include "ConverterBSON/XML/BSONToXML.h"
#include "Qido_rs.h"
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
    _contenttype(contenttype), _study_instance_uid_present(false),
    _series_instance_uid_present(false)
{
    mongo::BSONObj object = this->parse_string();

    this->add_mandatory_fields(object);

    QueryGenerator generator(this->_username);
    generator.set_includefields(this->_includefields);
    generator.set_maximumResults(this->_maximumResults);
    generator.set_skippedResults(this->_skippedResults);
    generator.set_fuzzymatching(this->_fuzzymatching);

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
    bool isfirst = true;
    while (findedobject.isValid() && !findedobject.isEmpty())
    {
        findedobject = this->check_mandatory_field_in_response(findedobject, generator);

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
            currentdata = replace(currentdata,
                                  "<?xml version=\"1.0\" encoding=\"utf-8\"?>",
                                  "<?xml version=\"1.0\" encoding=\"utf-8\" xml:space=\"preserve\" ?>");

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

    this->_study_instance_uid_present = study_instance_uid != "";
    if (study_instance_uid != "")
    {
        db_query << "0020000d" << BSON("vr" << "UI" <<
                                       "Value" << BSON_ARRAY(study_instance_uid));
    }

    this->_series_instance_uid_present = series_instance_uid != "";
    if (series_instance_uid != "")
    {
        db_query << "0020000e" << BSON("vr" << "UI" <<
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
            this->add_to_builder(tempbuilder, data[1], "");
            this->_includefields.push_back(tempbuilder.obj().firstElementFieldName());
        }
        else if (tag == "limit")
        {
            this->_maximumResults = std::atoi(data[1].c_str());
        }
        else if (tag == "offset")
        {
            this->_skippedResults = std::atoi(data[1].c_str());
            if (this->_skippedResults < 0)
            {
                this->_skippedResults = 0;
            }
        }
        else if (tag == "fuzzymatching")
        {
            // Not supported
            this->_fuzzymatching = (data[1] == "true");
        }
        else
        {
            this->add_to_builder(db_query, tag, data[1]);
        }
    }
    arraytemp.clear();

    if (!db_query.hasField("00080052"))
    {
        db_query << "00080052" << BSON("vr" << "CS" <<
                                       "Value" << BSON_ARRAY(this->_query_retrieve_level));
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

    if (response.getVR().getEVR() == EVR_PN)
    {
        builder << groupelement.str()
                << BSON("vr" << response.getVR().getVRName() <<
                        "Value" << BSON_ARRAY(BSON("Alphabetic" << value)));
    }
    else if (response.getVR().getEVR() == EVR_OB || response.getVR().getEVR() == EVR_OF ||
             response.getVR().getEVR() == EVR_OW || response.getVR().getEVR() == EVR_UN)
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

std::vector<Attribute> Qido_rs::get_mandatory_fields() const
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
::add_mandatory_fields(const mongo::BSONObj &queryobject)
{
    // Add tags
    for (Attribute tag : this->get_mandatory_fields())
    {
        // tag not already added
        if (std::find(this->_includefields.begin(),
                      this->_includefields.end(), tag._tag) != this->_includefields.end() ||
            queryobject.hasField(tag._tag))
        {
            continue;
        }

        this->_includefields.push_back(tag._tag);
    }
}

mongo::BSONObj
Qido_rs
::check_mandatory_field_in_response(const mongo::BSONObj &response,
                                    QueryGenerator &generator)
{
    mongo::BSONObjBuilder builderfinal;
    builderfinal.appendElements(response);

    // Add Query retrieve level
    mongo::BSONObj queryretrievelevel =
            BSON("00080052" << BSON("vr" << "CS" <<
                                    "Value" << BSON_ARRAY(this->_query_retrieve_level)));
    builderfinal.appendElements(queryretrievelevel);

    for (Attribute attribute : this->get_mandatory_fields())
    {
        if (attribute._tag == "00080052") continue;
        if (!response.hasField(attribute._tag))
        {
            if (attribute._tag == "00080056") // Instance Availability
            {
                builderfinal.appendElements(generator.compute_attribute(attribute._tag, ""));
            }
            else if (attribute._tag == "00080061") // Modalities in Study
            {
                std::string const value =
                        response["0020000d"].Obj().getField("Value").Array()[0].String();
                builderfinal.appendElements(generator.compute_attribute(attribute._tag, value));
            }
            else if (attribute._tag == "00201206") // Number of Study Related Series
            {
                std::string const value =
                        response["0020000d"].Obj().getField("Value").Array()[0].String();
                builderfinal.appendElements(generator.compute_attribute(attribute._tag, value));
            }
            else if (attribute._tag == "00201208") // Number of Study Related Instances
            {
                std::string const value =
                        response["0020000d"].Obj().getField("Value").Array()[0].String();
                builderfinal.appendElements(generator.compute_attribute(attribute._tag, value));
            }
            else if (attribute._tag == "00201209") // Number of Series Related Instances
            {
                std::string const value =
                        response["0020000e"].Obj().getField("Value").Array()[0].String();
                builderfinal.appendElements(generator.compute_attribute(attribute._tag, value));
            }
            else
            {
                mongo::BSONObjBuilder builder;
                builder << "vr" << attribute._vr;
                builder.appendNull("Value");

                mongo::BSONObj element =
                        BSON(attribute._tag << builder.obj());

                builderfinal.appendElements(element);
            }
        }
    }

    return builderfinal.obj();
}

} // namespace services

} // namespace dopamine
