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

// See PS3.18 - 6.7.1.2.2.1 Study Result Attributes
std::vector<std::string> const mandatory_study_attributes =
{
    "00080020", // Study Date
    "00080030", // Study Time
    "00080050", // Accession Number
    "00080090", // Referring Physician's Name
    "00100010", // Patient's Name
    "00100020", // Patient ID
    "00100030", // Patient's Birth Date
    "00100040", // Patient's Sex
    "0020000d", // Study Instance UID
    "00200010"  // Study ID
};

// See PS3.18 - 6.7.1.2.2.2 Series Result Attributes
std::vector<std::string> const mandatory_series_attributes =
{
    "00080060", // Modality
    "0008103e", // Series Description
    "0020000e", // Series Instance UID
    "00200011"  // Series Number
};

// See PS3.18 - 6.7.1.2.2.3 Instance Result Attributes
std::vector<std::string> const mandatory_instance_attributes =
{
    "00080016", // SOP Class UID
    "00080018", // SOP Instance UID
    "00080056", // Instance Availability
    "00200013"  // Instance Number
};

class Qido_rs : public Webservices
{
public:
    Qido_rs(std::string const & pathinfo,
            std::string const & querystring,
            std::string const & contenttype = MIME_TYPE_APPLICATION_DICOMXML,
            std::string const & remoteuser = "");

    ~Qido_rs();

    std::string get_contenttype() const;

protected:
    std::string _query_retrieve_level;

    std::vector<std::string> _includefields;

    std::string _contenttype;

    bool _study_instance_uid_present;

    bool _series_instance_uid_present;

private:
    virtual mongo::BSONObj parse_string();

    void add_to_builder(mongo::BSONObjBuilder & builder,
                        std::string const & tag,
                        std::string const & value);

    void add_mandatory_fields(mongo::BSONObj const & queryobject);

};

} // namespace services

} // namespace dopamine

#endif // _bfc0e3cc_01f0_402b_9e96_a8c825466940
