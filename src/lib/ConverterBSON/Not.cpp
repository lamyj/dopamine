/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "core/ExceptionPACS.h"
#include "Not.h"

namespace research_pacs
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
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    if (element == NULL)
    {
        throw research_pacs::ExceptionPACS("element is NULL.");
    }
    return !(*this->_condition)(element);
}

} // namespace research_pacs
