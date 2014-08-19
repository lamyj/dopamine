#include "VRMatch.h"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcvr.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "core/ExceptionPACS.h"

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
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    if (element == NULL)
    {
        throw research_pacs::ExceptionPACS("element is NULL.");
    }
    
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
