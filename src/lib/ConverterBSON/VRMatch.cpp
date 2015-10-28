/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "VRMatch.h"

#include <dcmtk/dcmdata/dcelem.h>

namespace dopamine
{

namespace converterBSON
{

VRMatch::Pointer
VRMatch
::New(dcmtkpp::VR vr)
{
    return Pointer(new VRMatch(vr));
}

VRMatch
::VRMatch(dcmtkpp::VR vr):
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
::operator()(dcmtkpp::Tag const & tag,
             dcmtkpp::Element const & element) const
    throw(dopamine::ExceptionPACS)
{
    return (element.vr == this->_vr);
}

} // namespace converterBSON

} // namespace dopamine
