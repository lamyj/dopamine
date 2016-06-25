/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _bfc0e3cc_01f0_402b_9e96_a8c825466940
#define _bfc0e3cc_01f0_402b_9e96_a8c825466940

#include <odil/registry.h>
#include <odil/Tag.h>
#include <odil/VR.h>

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
    Attribute(odil::Tag const & tag, odil::VR const & vr):
        _tag(tag), _vr(vr) {}

    odil::Tag get_tag() const
        { return this->_tag; }

    odil::VR get_vr() const
        { return this->_vr; }

private:
    odil::Tag _tag;
    odil::VR _vr;

};

// See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
std::vector<Attribute> const mandatory_study_attributes =
{
    Attribute(odil::registry::StudyDate, odil::VR::DA), // Study Date
    Attribute(odil::registry::StudyTime, odil::VR::TM), // Study Time
    Attribute(odil::registry::AccessionNumber, odil::VR::SH), // Accession Number
    Attribute(odil::registry::InstanceAvailability, odil::VR::CS), // Instance Availability
    Attribute(odil::registry::ModalitiesInStudy, odil::VR::CS), // Modalities in Study
    Attribute(odil::registry::ReferringPhysicianName, odil::VR::PN), // Referring Physician's Name
    Attribute(odil::registry::PatientName, odil::VR::PN), // Patient's Name
    Attribute(odil::registry::PatientID, odil::VR::LO), // Patient ID
    Attribute(odil::registry::PatientBirthDate, odil::VR::DA), // Patient's Birth Date
    Attribute(odil::registry::PatientSex, odil::VR::CS), // Patient's Sex
    Attribute(odil::registry::StudyInstanceUID, odil::VR::UI), // Study Instance UID
    Attribute(odil::registry::StudyID, odil::VR::SH), // Study ID
    Attribute(odil::registry::NumberOfStudyRelatedSeries, odil::VR::IS), // Number of Study Related Series
    Attribute(odil::registry::NumberOfStudyRelatedInstances, odil::VR::IS)  // Number of Study Related Instances
};

// See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
std::vector<Attribute> const mandatory_series_attributes =
{
    Attribute(odil::registry::Modality, odil::VR::CS), // Modality
    Attribute(odil::registry::SeriesDescription, odil::VR::LO), // Series Description
    Attribute(odil::registry::SeriesInstanceUID, odil::VR::UI), // Series Instance UID
    Attribute(odil::registry::SeriesNumber, odil::VR::IS), // Series Number
    Attribute(odil::registry::NumberOfSeriesRelatedInstances, odil::VR::IS)  // Number of Series Related Instances
};

// See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
std::vector<Attribute> const mandatory_instance_attributes =
{
    Attribute(odil::registry::SOPClassUID, odil::VR::UI), // SOP Class UID
    Attribute(odil::registry::SOPInstanceUID, odil::VR::UI), // SOP Instance UID
    Attribute(odil::registry::InstanceAvailability, odil::VR::CS), // Instance Availability
    Attribute(odil::registry::InstanceNumber, odil::VR::IS), // Instance Number
    Attribute(odil::registry::Rows, odil::VR::US), // Rows
    Attribute(odil::registry::Columns, odil::VR::US), // Columns
    Attribute(odil::registry::BitsAllocated, odil::VR::US)  // Bits Allocated
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
    virtual ~Qido_rs();

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
