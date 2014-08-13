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
    
class FindSCP : public SCP
{
public:
    FindSCP(T_ASC_Association * assoc, 
            T_ASC_PresentationContextID presID,
            T_DIMSE_C_FindRQ * req);
    
    virtual ~FindSCP();
    
    OFCondition process();

protected:

private:
    T_DIMSE_C_FindRQ * _request;

};
    
struct FindCallbackData
{
    FindSCP * scp;
    std::string ae_title;
};

} // namespace research_pacs

#endif // _c3674b2f_3f18_4264_89db_24dd4aaec99a
