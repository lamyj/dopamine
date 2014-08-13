/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _6f9b5539_5d0b_4f90_8779_314e21a8661f
#define _6f9b5539_5d0b_4f90_8779_314e21a8661f

#include "SCP.h"

namespace research_pacs
{
    
class EchoSCP : public SCP
{
public:
    EchoSCP(T_ASC_Association * assoc, 
            T_ASC_PresentationContextID presID,
            T_DIMSE_C_EchoRQ * req);
    
    virtual ~EchoSCP();
    
    OFCondition process();

protected:

private:
    T_DIMSE_C_EchoRQ * _request;

};
    
} // namespace research_pacs

#endif // _6f9b5539_5d0b_4f90_8779_314e21a8661f
