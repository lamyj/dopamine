#include "AlwaysFalse.h"

#include <dcmtk/config/osconfig.h>

AlwaysFalse::Pointer
AlwaysFalse
::New()
{
    return Pointer(new AlwaysFalse());
}

bool
AlwaysFalse
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    return false;
}

