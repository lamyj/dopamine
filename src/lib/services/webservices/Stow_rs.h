/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _f3f03ce9_754b_4843_9c32_7b7000d94934
#define _f3f03ce9_754b_4843_9c32_7b7000d94934

#include "services/webservices/Webservices.h"

namespace dopamine
{

namespace services
{

// see PS3.18 6.6 STOW-RS Request/Response
/**
 * @brief \class The Stow_rs class
 */
class Stow_rs : public Webservices
{
public:
    /**
     * @brief Create an instance of Stow_rs
     * @param pathinfo
     * @param querystring
     * @param postdata
     * @param remoteuser
     */
    Stow_rs(std::string const & pathinfo,
            std::string const & querystring,
            std::string const & postdata,
            std::string const & remoteuser = "");

    /// Destroy the instance of Stow_rs
    ~Stow_rs();

    std::string get_content_type() const;

    unsigned int get_status() const;

    std::string get_code() const;

protected:

private:
    virtual mongo::BSONObj _parse_string();

    std::string _find_content_type(std::string const & contenttype);

    void _process(std::string const & postdata,
                  mongo::BSONObj const & studyinstanceuid);

    std::string _content_type;

    unsigned int _status;

    std::string _code;

};

} // namespace services

} // namespace dopamine

#endif // _f3f03ce9_754b_4843_9c32_7b7000d94934
