/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "AlwaysFalse.h"

namespace dopamine
{

namespace converterBSON
{

AlwaysFalse
::AlwaysFalse()
: Condition()
{
    // Nothing to do
}

AlwaysFalse
::~AlwaysFalse()
{
    // Nothing to do
}

bool
AlwaysFalse
::operator()(
    odil::Tag const &, odil::Element const &) const
{
    return false;
}

} // namespace converterBSON

} // namespace dopamine
