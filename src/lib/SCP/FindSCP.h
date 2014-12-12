/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _c3674b2f_3f18_4264_89db_24dd4aaec99a
#define _c3674b2f_3f18_4264_89db_24dd4aaec99a

#include "SCP.h"

namespace research_pacs
{
    
/**
 * @brief SCP for C-FIND services
 */
class FindSCP : public SCP
{
public:
    /**
     * Create a default FindSCP
     * @param assoc: linked association
     * @param presID: linked presentation context
     * @param req: C-FIND request
     */
    FindSCP(T_ASC_Association * assoc, 
            T_ASC_PresentationContextID presID,
            T_DIMSE_C_FindRQ * req);
    
    /// Destroy the SCP
    virtual ~FindSCP();
    
    /**
     * Send the C-FIND response
     * @return EC_Normal if successful, an error code otherwise 
     */
    virtual OFCondition process();

protected:

private:
    /// Associated C-FIND request
    T_DIMSE_C_FindRQ * _request;

};

} // namespace research_pacs

#endif // _c3674b2f_3f18_4264_89db_24dd4aaec99a
