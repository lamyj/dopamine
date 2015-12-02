/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _e13d9a6a_5c07_47d7_be2e_ef276f943372
#define _e13d9a6a_5c07_47d7_be2e_ef276f943372

#include "SCPDispatcher.h"

#include <map>
#include <memory>

#include <dcmtkpp/SCP.h>
#include <dcmtkpp/Value.h>

namespace dopamine
{

template<typename TSCP>
void
SCPDispatcher
::set_scp(dcmtkpp::Value::Integer command, TSCP const & scp)
{
    SCPPointer pointer(new TSCP(scp));
    this->_providers[command] = pointer;
}

}

#endif // _e13d9a6a_5c07_47d7_be2e_ef276f943372
