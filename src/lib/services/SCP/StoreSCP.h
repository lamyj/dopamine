/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _57ab17de_94cf_44f4_8311_2a22f7360f34
#define _57ab17de_94cf_44f4_8311_2a22f7360f34

#include "services/SCP/SCP.h"

namespace dopamine
{

namespace services
{
    
/**
 * @brief SCP for C-STORE services
 */
class StoreSCP : public SCP
{
public:
    /**
     * Create a default StoreSCP
     * @param assoc: linked association
     * @param presID: linked presentation context
     * @param req: C-STORE request
     */
    StoreSCP(T_ASC_Association * association,
             T_ASC_PresentationContextID presentation_context_id,
             T_DIMSE_C_StoreRQ * request);
    
    /// Destroy the SCP
    virtual ~StoreSCP();
    
    /**
     * Send the C-STORE response
     * @return EC_Normal if successful, an error code otherwise 
     */
    virtual OFCondition process();

protected:

private:
    /// Associated C-STORE request
    T_DIMSE_C_StoreRQ * _request;
    
};

} // namespace services
    
} // namespace dopamine

#endif // _57ab17de_94cf_44f4_8311_2a22f7360f34
