/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _ee9915a2_a504_4b21_8d43_7938c66c526e
#define _ee9915a2_a504_4b21_8d43_7938c66c526e

#include "FindSCP.h"
#include "ResponseGenerator.h"

namespace research_pacs
{

class FindResponseGenerator : public ResponseGenerator
{
public :
    typedef FindResponseGenerator Self;

    FindResponseGenerator(FindSCP* scp, std::string const & ouraetitle);
    
    virtual ~FindResponseGenerator();
    
    void callBackHandler(
        /* in */
        OFBool cancelled, T_DIMSE_C_FindRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_FindRSP* response, DcmDataset** stDetail,
        DcmDataset** responseIdentifiers);
    
protected:
    virtual void next(DcmDataset ** responseIdentifiers);

private :
    bool _convert_modalities_in_study;
    
};

} // namespace research_pacs

#endif // _ee9915a2_a504_4b21_8d43_7938c66c526e
