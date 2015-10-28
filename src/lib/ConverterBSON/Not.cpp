/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "Not.h"

namespace dopamine
{

namespace converterBSON
{

Not::Pointer
Not
::New(Condition_DEBUG_RLA::Pointer const & condition)
{
    return Pointer(new Not(condition));
}

Not
::Not(Condition_DEBUG_RLA::Pointer const & condition):
    Condition_DEBUG_RLA(), _condition(condition)
{
    // Nothing else
}

Not
::~Not()
{
    // Nothing to do
}

bool
Not
::operator()(dcmtkpp::Tag const & tag,
             dcmtkpp::Element const & element) const
    throw(dopamine::ExceptionPACS)
{
    return !(*this->_condition)(tag, element);
}

} // namespace converterBSON

} // namespace dopamine
