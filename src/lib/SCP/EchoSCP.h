/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _6f9b5539_5d0b_4f90_8779_314e21a8661f
#define _6f9b5539_5d0b_4f90_8779_314e21a8661f

#include "SCP.h"

namespace dopamine
{
    
/**
 * @brief SCP for C-ECHO services
 */
class EchoSCP : public SCP
{
public:
    /**
     * Create a default EchoSCP
     * @param assoc: linked association
     * @param presID: linked presentation context
     * @param req: C-ECHO request
     */
    EchoSCP(T_ASC_Association * assoc, 
            T_ASC_PresentationContextID presID,
            T_DIMSE_C_EchoRQ * req);
    
    /// Destroy the SCP
    virtual ~EchoSCP();
    
    /**
     * Send the C-ECHO response
     * @return EC_Normal if successful, an error code otherwise 
     */
    virtual OFCondition process();

protected:

private:
    /// Associated C-ECHO request
    T_DIMSE_C_EchoRQ * _request;

};
    
} // namespace dopamine

#endif // _6f9b5539_5d0b_4f90_8779_314e21a8661f
