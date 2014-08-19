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
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    if (element == NULL)
    {
        throw research_pacs::ExceptionPACS("element is NULL.");
    }
    return (element->getGTag()%2 == 1);
}
