/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "core/ExceptionPACS.h"
#include "VRMatch.h"

VRMatch::Pointer
VRMatch
::New(DcmEVR vr)
{
    return Pointer(new VRMatch(vr));
}

VRMatch
::VRMatch(DcmEVR vr):
    Condition(), _vr(vr)
{
    // Nothing else.
}

VRMatch
::~VRMatch()
{
    // Nothing to do
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
    if(element->getVR() == this->_vr)
    {
        match = true;
    }
    else
    {
        // Try using DMCTK interval VRs
        if(DcmVR(element->getVR()).getValidEVR() == this->_vr)
        {
            match = true;
        }
    }
    return match;
}
