/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _01db2e6d_df7c_4b7a_ae0e_e04d2896413b
#define _01db2e6d_df7c_4b7a_ae0e_e04d2896413b

#include <odil/VR.h>

#include "Condition.h"

namespace dopamine
{

namespace converterBSON
{

/// @brief Condition matching a specific VR.
class VRMatch : public Condition
{
public:
    /// @brief Constructor.
    VRMatch(odil::VR vr);

    /// @brief Destructor.
    virtual ~VRMatch();

    /// @brief Test whether the element has the stored VR.
    virtual bool operator()(
        odil::Tag const & tag, odil::Element const & element) const;

private:
    odil::VR _vr;

};

} // namespace converterBSON

} // namespace dopamine

#endif // _01db2e6d_df7c_4b7a_ae0e_e04d2896413b
