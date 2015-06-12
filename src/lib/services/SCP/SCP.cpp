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
::SCP(T_ASC_Association * association,
      T_ASC_PresentationContextID presentation_context_id):
    _association(association),
    _presentation_context_id(presentation_context_id)
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
