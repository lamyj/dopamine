/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _7e2166a1_25b3_48eb_8226_abe9d64ba064
#define _7e2166a1_25b3_48eb_8226_abe9d64ba064

#include "services/RetrieveResponseGenerator.h"
#include "services/SCP/SCP.h"

namespace dopamine
{

namespace services
{
    
/**
 * @brief SCP for C-MOVE services
 */
class MoveSCP : public services::SCP
{
public:
    /**
     * Create a default MoveSCP
     * @param assoc: linked association
     * @param presID: linked presentation context
     * @param req: C-STORE request
     */
    MoveSCP(T_ASC_Association * assoc, 
            T_ASC_PresentationContextID presID,
            T_DIMSE_C_MoveRQ * req);
    
    /// Destroy the SCP
    virtual ~MoveSCP();
    
    /**
     * Send the C-MOVE response
     * @return EC_Normal if successful, an error code otherwise 
     */
    virtual OFCondition process();

    void set_network(T_ASC_Network *network);

protected:

private:
    /// Associated C-MOVE request
    T_DIMSE_C_MoveRQ * _request;

    T_ASC_Network * _network;
    
};

} // namespace services
    
} // namespace dopamine

#endif // _7e2166a1_25b3_48eb_8226_abe9d64ba064
