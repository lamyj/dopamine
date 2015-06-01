/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _f3f03ce9_754b_4843_9c32_7b7000d94934
#define _f3f03ce9_754b_4843_9c32_7b7000d94934

#include <string>
#include <vector>

#include "Webservices.h"

namespace dopamine
{

namespace services
{

// see PS3.18 6.6 STOW-RS Request/Response
class Stow_rs : public Webservices
{
public:
    Stow_rs(std::string const & pathinfo,
            std::string const & querystring,
            std::string const & postdata,
            std::string const & remoteuser = "");

    ~Stow_rs();

    std::string get_content_type() const;

    unsigned int get_status() const;

    std::string get_code() const;

protected:

private:
    virtual mongo::BSONObj parse_string();

    std::string find_content_type(std::string const & contenttype);

    void process(std::string const & postdata, mongo::BSONObj const & studyinstanceuid);

    std::string _content_type;

    unsigned int _status;

    std::string _code;

};

} // namespace services

} // namespace dopamine

#endif // _f3f03ce9_754b_4843_9c32_7b7000d94934
