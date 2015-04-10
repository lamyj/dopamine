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

private:
    virtual mongo::BSONObj parse_string();

    void add_to_builder(mongo::BSONObjBuilder & builder,
                        std::string const & tag,
                        std::string const & value);

};

} // namespace services

} // namespace dopamine

#endif // _bfc0e3cc_01f0_402b_9e96_a8c825466940
