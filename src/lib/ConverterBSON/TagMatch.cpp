#include "TagMatch.h"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcvr.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "core/ExceptionPACS.h"

TagMatch::Pointer
TagMatch
::New(DcmTagKey tag)
{
    return Pointer(new TagMatch(tag));
}

TagMatch
::TagMatch(DcmTagKey tag)
: tag(tag)
{
    // Nothing else.
}

bool
TagMatch
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    if (element == NULL)
    {
        throw research_pacs::ExceptionPACS("element is NULL.");
    }
    return (DcmTagKey(element->getGTag(), element->getETag()) == this->tag);
}

