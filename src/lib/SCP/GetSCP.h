/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _9514edbc_0abe_4f43_bf55_f064fb974d2e
#define _9514edbc_0abe_4f43_bf55_f064fb974d2e

#include "SCP.h"

namespace research_pacs
{
    
class GetSCP : public SCP
{
public:
    GetSCP(T_ASC_Association * assoc, 
           T_ASC_PresentationContextID presID,
           T_DIMSE_C_GetRQ * req);
    
    virtual ~GetSCP();
    
    OFCondition process();

protected:

private:
    T_DIMSE_C_GetRQ * _request;

};
    
} // namespace research_pacs

#endif // _9514edbc_0abe_4f43_bf55_f064fb974d2e
