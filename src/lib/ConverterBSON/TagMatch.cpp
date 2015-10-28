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

TagMatch::Pointer
TagMatch
::New(dcmtkpp::Tag tag)
{
    return Pointer(new TagMatch(tag));
}

TagMatch
::TagMatch(dcmtkpp::Tag tag):
    Condition_DEBUG_RLA(), _tag(tag)
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
::operator()(dcmtkpp::Tag const & tag,
             dcmtkpp::Element const & element) const
    throw(dopamine::ExceptionPACS)
{
    return (tag == this->_tag);
}

} // namespace converterBSON

} // namespace dopamine
