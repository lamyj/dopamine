/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _23d72937_8451_4cd1_a64d_77b774672145
#define _23d72937_8451_4cd1_a64d_77b774672145

#include "Wado.h"

namespace dopamine
{

namespace services
{

class Wado_rs: public Wado
{
public:
    Wado_rs(std::string const & pathinfo,
            std::string const & remoteuser = "");

    ~Wado_rs();

protected:

private:
    virtual mongo::BSONObj parse_string();

};

} // namespace services

} // namespace dopamine

#endif // _23d72937_8451_4cd1_a64d_77b774672145
