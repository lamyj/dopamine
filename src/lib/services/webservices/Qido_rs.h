/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _bfc0e3cc_01f0_402b_9e96_a8c825466940
#define _bfc0e3cc_01f0_402b_9e96_a8c825466940

#include <dcmtkpp/registry.h>
#include <dcmtkpp/Tag.h>
#include <dcmtkpp/VR.h>

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
    Attribute(dcmtkpp::Tag const & tag, dcmtkpp::VR const & vr):
        _tag(tag), _vr(vr) {}

    dcmtkpp::Tag get_tag() const
        { return this->_tag; }

    dcmtkpp::VR get_vr() const
        { return this->_vr; }

private:
    dcmtkpp::Tag _tag;
    dcmtkpp::VR _vr;

};

// See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
std::vector<Attribute> const mandatory_study_attributes =
{
    Attribute(dcmtkpp::registry::StudyDate, dcmtkpp::VR::DA), // Study Date
    Attribute(dcmtkpp::registry::StudyTime, dcmtkpp::VR::TM), // Study Time
    Attribute(dcmtkpp::registry::AccessionNumber, dcmtkpp::VR::SH), // Accession Number
    Attribute(dcmtkpp::registry::InstanceAvailability, dcmtkpp::VR::CS), // Instance Availability
    Attribute(dcmtkpp::registry::ModalitiesInStudy, dcmtkpp::VR::CS), // Modalities in Study
    Attribute(dcmtkpp::registry::ReferringPhysicianName, dcmtkpp::VR::PN), // Referring Physician's Name
    Attribute(dcmtkpp::registry::PatientName, dcmtkpp::VR::PN), // Patient's Name
    Attribute(dcmtkpp::registry::PatientID, dcmtkpp::VR::LO), // Patient ID
    Attribute(dcmtkpp::registry::PatientBirthDate, dcmtkpp::VR::DA), // Patient's Birth Date
    Attribute(dcmtkpp::registry::PatientSex, dcmtkpp::VR::CS), // Patient's Sex
    Attribute(dcmtkpp::registry::StudyInstanceUID, dcmtkpp::VR::UI), // Study Instance UID
    Attribute(dcmtkpp::registry::StudyID, dcmtkpp::VR::SH), // Study ID
    Attribute(dcmtkpp::registry::NumberOfStudyRelatedSeries, dcmtkpp::VR::IS), // Number of Study Related Series
    Attribute(dcmtkpp::registry::NumberOfStudyRelatedInstances, dcmtkpp::VR::IS)  // Number of Study Related Instances
};

// See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
std::vector<Attribute> const mandatory_series_attributes =
{
    Attribute(dcmtkpp::registry::Modality, dcmtkpp::VR::CS), // Modality
    Attribute(dcmtkpp::registry::SeriesDescription, dcmtkpp::VR::LO), // Series Description
    Attribute(dcmtkpp::registry::SeriesInstanceUID, dcmtkpp::VR::UI), // Series Instance UID
    Attribute(dcmtkpp::registry::SeriesNumber, dcmtkpp::VR::IS), // Series Number
    Attribute(dcmtkpp::registry::NumberOfSeriesRelatedInstances, dcmtkpp::VR::IS)  // Number of Series Related Instances
};

// See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
std::vector<Attribute> const mandatory_instance_attributes =
{
    Attribute(dcmtkpp::registry::SOPClassUID, dcmtkpp::VR::UI), // SOP Class UID
    Attribute(dcmtkpp::registry::SOPInstanceUID, dcmtkpp::VR::UI), // SOP Instance UID
    Attribute(dcmtkpp::registry::InstanceAvailability, dcmtkpp::VR::CS), // Instance Availability
    Attribute(dcmtkpp::registry::InstanceNumber, dcmtkpp::VR::IS), // Instance Number
    Attribute(dcmtkpp::registry::Rows, dcmtkpp::VR::US), // Rows
    Attribute(dcmtkpp::registry::Columns, dcmtkpp::VR::US), // Columns
    Attribute(dcmtkpp::registry::BitsAllocated, dcmtkpp::VR::US)  // Bits Allocated
};

/**
 * @brief \class The Qido_rs class
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
    std::string _contenttype;

    bool _study_instance_uid_present;

    bool _series_instance_uid_present;

private:
    /// Fields to retrieve
    std::vector<std::string> _include_fields;

    /// Maximum number of dataset to retrieve
    int _maximum_results;

    /// Number of response to ignore
    int _skipped_results;

    bool _fuzzy_matching;

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
