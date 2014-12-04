/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h>

#include "AlwaysFalse.h"

namespace research_pacs
{

AlwaysFalse::Pointer
AlwaysFalse
::New()
{
    return Pointer(new AlwaysFalse());
}

AlwaysFalse
::AlwaysFalse():
    Condition()
{
    // Nothing to do
}

AlwaysFalse
::~AlwaysFalse()
{
    // Nothing to do
}

bool
AlwaysFalse
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    return false;
}

} // namespace research_pacs
