/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "AlwaysTrue.h"

namespace dopamine
{

namespace converterBSON
{

AlwaysTrue
::AlwaysTrue()
: Condition()
{
    // Nothing to do
}

AlwaysTrue
::~AlwaysTrue()
{
    // Nothing to do
}

bool
AlwaysTrue
::operator()(
    odil::Tag const &, odil::Element const &) const
{
    return true;
}

} // namespace converterBSON

} // namespace dopamine
