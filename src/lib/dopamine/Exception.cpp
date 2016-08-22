/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/Exception.h"

#include <stdexcept>
#include <string>

namespace dopamine
{

Exception
::Exception(std::string const & message)
: _message(message)
{
    // Nothing else.
}

Exception
::~Exception() noexcept
{
    // Nothing to do.
}

char const *
Exception
::what() const noexcept
{
    return this->_message.c_str();
}

}
