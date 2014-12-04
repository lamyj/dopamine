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
#include "IsPrivateTag.h"

namespace research_pacs
{

IsPrivateTag::Pointer
IsPrivateTag
::New()
{
    return Pointer(new IsPrivateTag());
}

IsPrivateTag
::IsPrivateTag():
    Condition()
{
    // Nothing else
}

IsPrivateTag
::~IsPrivateTag()
{
    // Nothing to do
}

bool
IsPrivateTag
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    if (element == NULL)
    {
        throw research_pacs::ExceptionPACS("element is NULL.");
    }
    return (element->getGTag()%2 == 1);
}

} // namespace research_pacs
