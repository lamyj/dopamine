#include "IsPrivateTag.h"

#include <boost/shared_ptr.hpp>
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

IsPrivateTag::Pointer
IsPrivateTag
::New()
{
    return Pointer(new IsPrivateTag());
}

IsPrivateTag
::IsPrivateTag()
{
    // Nothing else
}

bool
IsPrivateTag
::operator()(DcmElement * element) const
{
    return (element->getGTag()%2 == 1);
}
