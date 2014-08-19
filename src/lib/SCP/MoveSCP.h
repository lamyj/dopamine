/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _7e2166a1_25b3_48eb_8226_abe9d64ba064
#define _7e2166a1_25b3_48eb_8226_abe9d64ba064

#include "SCP.h"

namespace research_pacs
{
    
class MoveSCP : public SCP
{
public:
    MoveSCP(T_ASC_Association * assoc, 
            T_ASC_PresentationContextID presID,
            T_DIMSE_C_MoveRQ * req);
    
    virtual ~MoveSCP();
    
    OFCondition process();

protected:

private:
    T_DIMSE_C_MoveRQ * _request;
    
};
    
} // namespace research_pacs

#endif // _7e2166a1_25b3_48eb_8226_abe9d64ba064
