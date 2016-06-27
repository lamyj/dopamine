/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "Or.h"

#include <algorithm>

namespace dopamine
{

namespace converterBSON
{

Or
::Or():
    Condition()
{
    // Nothing else
}

Or
::~Or()
{
    // Nothing to do
}

bool
Or
::operator()(odil::Tag const & tag, odil::Element const & element) const
{
    return std::any_of(
        this->_conditions.begin(), this->_conditions.end(),
        [&tag,&element](std::shared_ptr<Condition> const & condition)
        {
            return (*condition)(tag, element);
        }
    );
}

std::vector<std::shared_ptr<Condition>> const &
Or
::get_terms() const
{
    return this->_conditions;
}

std::vector<std::shared_ptr<Condition>> &
Or
::get_terms()
{
    return this->_conditions;
}

} // namespace converterBSON

} // namespace dopamine
