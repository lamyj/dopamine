/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/regex.hpp>

#include <dcmtkpp/message/CFindResponse.h>

#include "ConverterBSON/bson_converter.h"
#include "core/dataset_tools.h"
#include "services/FindGenerator.h"
#include "services/ServicesTools.h"
#include "services/webservices/Qido_rs.h"
#include "services/webservices/WebServiceException.h"

namespace dopamine
{

namespace services
{

void
check_mandatory_field_in_response(dcmtkpp::DataSet & response,
                                  FindGenerator::Pointer generator,
                                  std::vector<Attribute> attributes)
{
    // Add Query retrieve level
    response.add(dcmtkpp::registry::QueryRetrieveLevel,
                 {generator->get_query_retrieve_level()}, dcmtkpp::VR::CS);

    for (Attribute attribute : attributes)
    {
        if (attribute.get_tag() == dcmtkpp::registry::QueryRetrieveLevel)
        {
            continue;
        }

        if (!response.has(attribute.get_tag()))
        {
            // Instance Availability
            if (attribute.get_tag() == dcmtkpp::registry::InstanceAvailability)
            {
                response.add(dcmtkpp::registry::InstanceAvailability,
                             generator->compute_attribute(attribute.get_tag(),
                                                          attribute.get_vr(),
                                                          ""));
            }
            // Modalities in Study
            else if (attribute.get_tag() == dcmtkpp::registry::ModalitiesInStudy)
            {
                std::string const value =
                    response.as_string(dcmtkpp::registry::StudyInstanceUID)[0];
                response.add(dcmtkpp::registry::ModalitiesInStudy,
                             generator->compute_attribute(attribute.get_tag(),
                                                          attribute.get_vr(),
                                                          value));
            }
            // Number of Study Related Series
            else if (attribute.get_tag() ==
                     dcmtkpp::registry::NumberOfStudyRelatedSeries)
            {
                std::string const value =
                    response.as_string(dcmtkpp::registry::StudyInstanceUID)[0];
                response.add(dcmtkpp::registry::NumberOfStudyRelatedSeries,
                             generator->compute_attribute(attribute.get_tag(),
                                                          attribute.get_vr(),
                                                          value));
            }
            // Number of Study Related Instances
            else if (attribute.get_tag() ==
                     dcmtkpp::registry::NumberOfStudyRelatedInstances)
            {
                std::string const value =
                    response.as_string(dcmtkpp::registry::StudyInstanceUID)[0];
                response.add(dcmtkpp::registry::NumberOfStudyRelatedInstances,
                             generator->compute_attribute(attribute.get_tag(),
                                                          attribute.get_vr(),
                                                          value));
            }
            // Number of Series Related Instances
            else if (attribute.get_tag() ==
                     dcmtkpp::registry::NumberOfSeriesRelatedInstances)
            {
                std::string const value =
                    response.as_string(dcmtkpp::registry::SeriesInstanceUID)[0];
                response.add(dcmtkpp::registry::NumberOfSeriesRelatedInstances,
                             generator->compute_attribute(attribute.get_tag(),
                                                          attribute.get_vr(),
                                                          value));
            }
            else
            {
                response.add(attribute.get_tag(), attribute.get_vr());
            }
        }
    }
}

Qido_rs
::Qido_rs(std::string const & pathinfo,
          std::string const & querystring,
          std::string const & contenttype,
          std::string const & remoteuser):
    Webservices(pathinfo, querystring),
    _contenttype(contenttype), _study_instance_uid_present(false),
    _series_instance_uid_present(false), _include_fields({}),
    _maximum_results(0), _skipped_results(0), _fuzzy_matching(false)
{
    FindGenerator::Pointer generator = FindGenerator::New();
    generator->set_username(remoteuser);

    mongo::BSONObj object = this->_parse_string();
    this->_add_mandatory_fields(object);

    generator->set_query_retrieve_level(this->_query_retrieve_level);
    generator->set_include_fields(this->_include_fields);
    generator->set_maximum_results(this->_maximum_results);
    generator->set_skipped_results(this->_skipped_results);
    generator->set_fuzzy_matching(this->_fuzzy_matching);

    auto status = generator->initialize(object);
    if (status != dcmtkpp::message::CFindResponse::Pending)
    {
        if (status == dcmtkpp::message::CFindResponse::RefusedNotAuthorized)
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

    if (generator->done())
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
    else
    {
        bool isfirst = true;
        while (!generator->done())
        {
            generator->next();

            dcmtkpp::DataSet dataset = generator->get().second;

            check_mandatory_field_in_response(dataset, generator,
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

                stream << dataset_to_json_string(dataset);
            }
            else if (this->_contenttype == MIME_TYPE_APPLICATION_DICOMXML)
            {
                stream << "--" << this->_boundary << "\n";
                stream << CONTENT_TYPE
                       << MIME_TYPE_APPLICATION_DICOMXML << "\n\n";

                std::string currentdata = dataset_to_xml_string(dataset);

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
        }
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

    if (!vartemp.empty() && vartemp[0] == "")
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
        this->_query_retrieve_level = "STUDY";
        if (vartemp.size() > 1)
        {
            study_instance_uid = vartemp[1];

            if (vartemp.size() > 2)
            {
                if (vartemp[2] == "series")
                {
                    this->_query_retrieve_level = "SERIES";
                    if (vartemp.size() > 3)
                    {
                        series_instance_uid = vartemp[3];

                        if (vartemp.size() > 4)
                        {
                            if (vartemp[4] == "instances")
                            {
                                this->_query_retrieve_level = "IMAGE";

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
                    this->_query_retrieve_level = "IMAGE";

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
        this->_query_retrieve_level = "SERIES";

        if (vartemp.size() > 1)
        {
            throw WebServiceException(400, "Bad Request",
                                      "too many parameters");
        }
    }
    else if (vartemp[0] == "instances")
    {
        this->_query_retrieve_level = "IMAGE";

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

    this->_include_fields.clear();
    for (std::string variable : arraytemp)
    {
        std::vector<std::string> data;
        boost::split(data, variable, boost::is_any_of("="));

        std::string tag = data[0];
        if (tag == "includefield")
        {
            mongo::BSONObjBuilder tempbuilder;
            this->_add_to_builder(tempbuilder, data[1], "");
            this->_include_fields.push_back(
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

    try
    {
        dcmtkpp::Tag dcmtkpptag(tag);
        std::stringstream groupelement;

        std::stringstream streamgroup;
        streamgroup << std::hex << dcmtkpptag.group;
        std::string group = streamgroup.str();
        while (group.length() != 4)
        {
            group = "0" + group;
        }

        std::stringstream streamelement;
        streamelement << std::hex << dcmtkpptag.element;
        std::string element = streamelement.str();
        while (element.length() != 4)
        {
            element = "0" + element;
        }

        groupelement << group << element;

        auto const vr = dcmtkpp::as_vr(dcmtkpptag);

        if (vr == dcmtkpp::VR::PN)
        {
            builder << groupelement.str()
                    << BSON("vr" << dcmtkpp::as_string(vr) <<
                            "Value" << BSON_ARRAY(BSON("Alphabetic" << value)));
        }
        else if (vr == dcmtkpp::VR::OB || vr == dcmtkpp::VR::OF ||
                 vr == dcmtkpp::VR::OW || vr == dcmtkpp::VR::UN)
        {
            builder << groupelement.str()
                    << BSON("vr" << dcmtkpp::as_string(vr) <<
                            "InlineBinary" << value);
        }
        else
        {
            builder << groupelement.str()
                    << BSON("vr" << dcmtkpp::as_string(vr) <<
                            "Value" << BSON_ARRAY(value));
        }
    }
    catch (dcmtkpp::Exception const & exc)
    {
        std::stringstream stream;
        stream << "Unknown DICOM Tag: " << tag;
        throw WebServiceException(400, "Bad Request",
                                  stream.str());
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
        if (std::find(this->_include_fields.begin(),
                      this->_include_fields.end(),
                      std::string(tag.get_tag())) !=
                this->_include_fields.end() ||
            queryobject.hasField(std::string(tag.get_tag())))
        {
            continue;
        }

        this->_include_fields.push_back(tag.get_tag());
    }
}

} // namespace services

} // namespace dopamine
