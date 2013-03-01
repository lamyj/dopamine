#include "AlwaysTrue.h"

#include <dcmtk/config/osconfig.h>

AlwaysTrue::Pointer
AlwaysTrue
::New()
{
    return Pointer(new AlwaysTrue());
}

bool
AlwaysTrue
::operator()(DcmElement * element) const
{
    return true;
}
