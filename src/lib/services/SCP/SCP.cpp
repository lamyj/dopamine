/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "SCP.h"

namespace dopamine
{

namespace services
{
    
SCP
::SCP(T_ASC_Association * assoc, T_ASC_PresentationContextID presID):
    _association(assoc), _presentationID(presID)
{
    // nothing to do
}

SCP
::~SCP()
{
    // nothing to do
}

} // namespace services
    
} // namespace dopamine
