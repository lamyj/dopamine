#include "VRMatch.h"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcvr.h>
#include <dcmtk/dcmdata/dcelem.h>

VRMatch::Pointer
VRMatch
::New(DcmEVR vr)
{
    return Pointer(new VRMatch(vr));
}

VRMatch
::VRMatch(DcmEVR vr)
: vr(vr)
{
    // Nothing else.
}

bool
VRMatch
::operator()(DcmElement * element) const
{
    bool match=false;
    if(element->getVR() == this->vr)
    {
        match = true;
    }
    else
    {
        // Try using DMCTK interval VRs
        if(DcmVR(element->getVR()).getValidEVR() == this->vr)
        {
            match = true;
        }
    }
    return match;
}
