/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _0df310f1_c98d_4b26_a02f_be846908e094
#define _0df310f1_c98d_4b26_a02f_be846908e094

#include "Condition.h"

namespace dopamine
{

namespace converterBSON
{

/// @brief Always True Condition
class AlwaysTrue : public Condition
{
public:
    /// @brief Constructor.
    AlwaysTrue();

    /// @brief Destructor.
    virtual ~AlwaysTrue();

    /// @brief Always return true.
    virtual bool operator()(
        odil::Tag const & tag, odil::Element const & element) const;
};

} // namespace converterBSON

} // namespace dopamine

#endif // _0df310f1_c98d_4b26_a02f_be846908e094
