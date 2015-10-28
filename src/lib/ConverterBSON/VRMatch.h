/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _01db2e6d_df7c_4b7a_ae0e_e04d2896413b
#define _01db2e6d_df7c_4b7a_ae0e_e04d2896413b

#include <dcmtkpp/VR.h>

#include "Condition.h"

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief \class VRMatch Condition
 */
class VRMatch : public Condition_DEBUG_RLA
{
public:
    typedef boost::shared_ptr<VRMatch> Pointer;
    
    /// Create pointer to new instance of VRMatch
    static Pointer New(dcmtkpp::VR vr);
    
    /// Destroy the instance of VRMatch
    virtual ~VRMatch();

    /**
     * Operator ()
     * @param element: tested element
     * @return true if element's VR match with Searched VR, false otherwise
     * @throw ExceptionPACS if element is null
     */
    virtual bool operator()(dcmtkpp::Tag const & tag,
                            dcmtkpp::Element const & element)
            const throw(dopamine::ExceptionPACS);
    
protected:

private:
    /// Create an instance of VRMatch
    VRMatch(dcmtkpp::VR vr);
    
    /// Compared VR
    dcmtkpp::VR _vr;

};

} // namespace converterBSON

} // namespace dopamine

#endif // _01db2e6d_df7c_4b7a_ae0e_e04d2896413b
