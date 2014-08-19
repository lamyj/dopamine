#include "IsPrivateTag.h"

#include <boost/shared_ptr.hpp>
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "core/ExceptionPACS.h"

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
    if (element == NULL)
    {
        throw research_pacs::ExceptionPACS("IsPrivateTag: tested element cannot be NULL.");
    }
    return (element->getGTag()%2 == 1);
}
