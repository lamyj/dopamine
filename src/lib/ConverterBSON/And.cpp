/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "And.h"

namespace dopamine
{

namespace converterBSON
{

And::Pointer
And
::New()
{
    return Pointer(new And());
}

And
::And():
    Condition()
{
    // Nothing else
}

And
::~And()
{
    // Nothing to do
}

bool
And
::operator()(dcmtkpp::Tag const & tag,
             dcmtkpp::Element const & element) const
    throw(dopamine::ExceptionPACS)
{
    bool value=true;
    for(auto it = this->_conditions.begin(); it != this->_conditions.end(); ++it)
    {
        value = value && (**it)(tag, element);
        if(!value)
        {
            break;
        }
    }

    return value;
}

void
And
::insert_condition(Condition::Pointer condition)
{
    this->_conditions.push_back(condition);
}

} // namespace converterBSON

} // namespace dopamine
