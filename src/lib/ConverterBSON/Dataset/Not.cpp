/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "core/ExceptionPACS.h"
#include "Not.h"

namespace dopamine
{

namespace converterBSON
{

Not::Pointer
Not
::New(Condition::Pointer const & condition)
{
    return Pointer(new Not(condition));
}

Not
::Not(Condition::Pointer const & condition):
    Condition(), _condition(condition)
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
::operator()(DcmElement * element) const throw(dopamine::ExceptionPACS)
{
    if (element == NULL)
    {
        throw dopamine::ExceptionPACS("element is NULL.");
    }
    return !(*this->_condition)(element);
}

} // namespace converterBSON

} // namespace dopamine
