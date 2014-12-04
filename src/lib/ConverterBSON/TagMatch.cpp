/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "TagMatch.h"

#include <dcmtk/dcmdata/dcelem.h>

namespace research_pacs
{

TagMatch::Pointer
TagMatch
::New(DcmTagKey tag)
{
    return Pointer(new TagMatch(tag));
}

TagMatch
::TagMatch(DcmTagKey tag): 
    Condition(), _tag(tag)
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
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    if (element == NULL)
    {
        throw research_pacs::ExceptionPACS("element is NULL.");
    }
    return (DcmTagKey(element->getGTag(), element->getETag()) == this->_tag);
}

} // namespace research_pacs
