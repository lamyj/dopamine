#include "TagMatch.h"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcvr.h>
#include <dcmtk/dcmdata/dcelem.h>

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
::operator()(DcmElement * element) const
{
    return (DcmTagKey(element->getGTag(), element->getETag()) == this->tag);
}

