/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "Or.h"

namespace dopamine
{

namespace converterBSON
{

Or::Pointer
Or
::New()
{
    return Pointer(new Or());
}

Or
::Or():
    Condition_DEBUG_RLA()
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
::operator()(dcmtkpp::Tag const & tag,
             dcmtkpp::Element const & element) const
    throw(dopamine::ExceptionPACS)
{
    bool value=false;
    for(auto it = this->_conditions.begin();
        it != this->_conditions.end(); ++it)
    {
        value = value || (**it)(tag, element);
        if(value)
        {
            break;
        }
    }

    return value;
}

void
Or
::insert_condition(Condition_DEBUG_RLA::Pointer condition)
{
    this->_conditions.push_back(condition);
}

} // namespace converterBSON

} // namespace dopamine
