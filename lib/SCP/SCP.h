/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _2a40efaa_eb3c_40f4_a8ba_e614ae1fb9f8
#define _2a40efaa_eb3c_40f4_a8ba_e614ae1fb9f8

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmnet/assoc.h"
#include "dcmtk/dcmnet/dimse.h"

namespace research_pacs
{
    
class SCP
{
public:
    /// Create a default.
    SCP(T_ASC_Association * assoc, T_ASC_PresentationContextID presID);
    
    /// Destroy the SCP.
    virtual ~SCP();
    
    T_ASC_Association* get_association() const { return this->_association; }
    
protected:
    mutable T_ASC_Association * _association;
    T_ASC_PresentationContextID _presentationID;

private:
    
};
    
} // namespace research_pacs

#endif // _2a40efaa_eb3c_40f4_a8ba_e614ae1fb9f8
