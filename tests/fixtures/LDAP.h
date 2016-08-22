/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _c5e22f2d_38ed_4b13_ba04_5cd3f89a3f65
#define _c5e22f2d_38ed_4b13_ba04_5cd3f89a3f65

#include <string>

/**
 * The following environment variables must be defined
 * * URI
 * * BIND_DN_TEMPLATE
 * * USERNAME
 * * PASSWORD
 */

namespace fixtures
{

struct LDAP
{
    std::string uri;
    std::string bind_dn_template;
    std::string username;
    std::string password;

    LDAP();

    ~LDAP();

    static std::string get_environment_variable(std::string const & name);
};

} // namespace fixtures

#endif // _c5e22f2d_38ed_4b13_ba04_5cd3f89a3f65

