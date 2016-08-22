/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _9e541e16_f795_4e11_9f80_6892bc67ae28
#define _9e541e16_f795_4e11_9f80_6892bc67ae28

#include <map>
#include <memory>
#include <string>

#include "dopamine/authentication/AuthenticatorBase.h"

namespace dopamine
{

namespace authentication
{

std::shared_ptr<AuthenticatorBase> factory(
    std::map<std::string, std::string> const & properties);

}

}

#endif // _9e541e16_f795_4e11_9f80_6892bc67ae28
