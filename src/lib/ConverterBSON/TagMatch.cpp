/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "TagMatch.h"

namespace dopamine
{

namespace converterBSON
{

TagMatch
::TagMatch(odil::Tag tag)
: Condition(), _tag(tag)
{
    // Nothing else.
}

TagMatch
::~TagMatch()
{
    // Nothing to do
}

bool
TagMatch
::operator()(odil::Tag const & tag, odil::Element const &) const
{
    return (tag == this->_tag);
}

} // namespace converterBSON

} // namespace dopamine
