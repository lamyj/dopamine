/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _5df462f6_0976_4a99_b597_0761a549979c
#define _5df462f6_0976_4a99_b597_0761a549979c

#include <map>

#include "Wado.h"

namespace dopamine
{

namespace services
{

/**
 * @brief The parameters struct
 */
struct parameters
{
public:
    parameters(bool mandatory, bool used):
        _mandatory(mandatory), _used(used) {}

    bool is_mandatory() const
            { return this->_mandatory; }

    bool is_used() const
            { return this->_used; }

private:
    bool _mandatory;
    bool _used;

};

// Mandatory Request Parameters
std::string const REQUEST_TYPE = "requestType";
std::string const STUDY_INSTANCE_UID = "studyUID";
std::string const SERIES_INSTANCE_UID = "seriesUID";
std::string const SOP_INSTANCE_UID = "objectUID";

// List of Request Parameters: see PS3.17 Table HHH.1-1
std::map<std::string, parameters> const RequestParameters = {
    { REQUEST_TYPE, parameters(true, true) },
    { STUDY_INSTANCE_UID, parameters(true, true) },
    { SERIES_INSTANCE_UID, parameters(true, true) },
    { SOP_INSTANCE_UID, parameters(true, true) },
    { "contentType", parameters(false, false) },
    { "charset", parameters(false, false) },
    { "anonymize", parameters(false, false) },
    { "annotation", parameters(false, false) },
    { "Rows", parameters(false, false) },
    { "Column", parameters(false, false) },
    { "region", parameters(false, false) },
    { "windowCenter", parameters(false, false) },
    { "windowWidth", parameters(false, false) },
    { "imageQuality", parameters(false, false) },
    { "presentationUID", parameters(false, false) },
    { "presentationSeriesUID", parameters(false, false) },
    { "transferSyntax", parameters(false, false) },
    { "frameNumber", parameters(false, false) }
};

/**
 * @brief The Wado_uri class
 */
class Wado_uri : public Wado
{
public:
    /**
     * @brief Create an instance of Wado_uri
     * @param querystring
     * @param remoteuser
     */
    Wado_uri(std::string const & querystring,
             std::string const & remoteuser = "");

    /// Destroy the instance of Wado_uri
    ~Wado_uri();

    std::string get_filename() const;

protected:

private:
    virtual mongo::BSONObj _parse_string();

    std::string _filename;

};

} // namespace services

} // namespace dopamine

#endif // _5df462f6_0976_4a99_b597_0761a549979c
