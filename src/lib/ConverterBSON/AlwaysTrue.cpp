/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h>

#include "AlwaysTrue.h"

namespace research_pacs
{

AlwaysTrue::Pointer
AlwaysTrue
::New()
{
    return Pointer(new AlwaysTrue());
}

AlwaysTrue
::AlwaysTrue():
    Condition()
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
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    return true;
}

} // namespace research_pacs
