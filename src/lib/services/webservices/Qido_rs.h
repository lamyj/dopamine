/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _bfc0e3cc_01f0_402b_9e96_a8c825466940
#define _bfc0e3cc_01f0_402b_9e96_a8c825466940

#include "Webservices.h"

namespace dopamine
{

namespace services
{

/**
 * @brief The Attribute struct
 */
struct Attribute
{
public:
    Attribute(std::string const & tag, std::string const & vr):
        _tag(tag), _vr(vr) {}

    std::string get_tag() const
        { return this->_tag; }

    std::string get_vr() const
        { return this->_vr; }

private:
    std::string _tag;
    std::string _vr;

};

// See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
std::vector<Attribute> const mandatory_study_attributes =
{
    Attribute("00080020", "DA"), // Study Date
    Attribute("00080030", "TM"), // Study Time
    Attribute("00080050", "SH"), // Accession Number
    Attribute("00080056", "CS"), // Instance Availability
    Attribute("00080061", "CS"), // Modalities in Study
    Attribute("00080090", "PN"), // Referring Physician's Name
    Attribute("00100010", "PN"), // Patient's Name
    Attribute("00100020", "LO"), // Patient ID
    Attribute("00100030", "DA"), // Patient's Birth Date
    Attribute("00100040", "CS"), // Patient's Sex
    Attribute("0020000d", "UI"), // Study Instance UID
    Attribute("00200010", "SH"), // Study ID
    Attribute("00201206", "IS"), // Number of Study Related Series
    Attribute("00201208", "IS")  // Number of Study Related Instances
};

// See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
std::vector<Attribute> const mandatory_series_attributes =
{
    Attribute("00080060", "CS"), // Modality
    Attribute("0008103e", "LO"), // Series Description
    Attribute("0020000e", "UI"), // Series Instance UID
    Attribute("00200011", "IS"), // Series Number
    Attribute("00201209", "IS")  // Number of Series Related Instances
};

// See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
std::vector<Attribute> const mandatory_instance_attributes =
{
    Attribute("00080016", "UI"), // SOP Class UID
    Attribute("00080018", "UI"), // SOP Instance UID
    Attribute("00080056", "CS"), // Instance Availability
    Attribute("00200013", "IS"), // Instance Number
    Attribute("00280010", "US"), // Rows
    Attribute("00280011", "US"), // Columns
    Attribute("00280100", "US")  // Bits Allocated
};

/**
 * @brief The Qido_rs class
 */
class Qido_rs : public Webservices
{
public:
    /**
     * @brief Create an instance of Qido_rs
     * @param pathinfo
     * @param querystring
     * @param contenttype
     * @param remoteuser
     */
    Qido_rs(std::string const & pathinfo,
            std::string const & querystring,
            std::string const & contenttype = MIME_TYPE_APPLICATION_DICOMXML,
            std::string const & remoteuser = "");

    /// Destroy the instance of Qido_rs
    ~Qido_rs();

    std::string get_contenttype() const;

protected:
    std::string _query_retrieve_level;

    std::vector<std::string> _includefields;

    std::string _contenttype;

    bool _study_instance_uid_present;

    bool _series_instance_uid_present;

private:
    virtual mongo::BSONObj _parse_string();

    void _add_to_builder(mongo::BSONObjBuilder & builder,
                         std::string const & tag,
                         std::string const & value);

    std::vector<Attribute> _get_mandatory_fields() const;

    void _add_mandatory_fields(mongo::BSONObj const & queryobject);

};

} // namespace services

} // namespace dopamine

#endif // _bfc0e3cc_01f0_402b_9e96_a8c825466940
