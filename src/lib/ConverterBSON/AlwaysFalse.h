/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _4acc859b_9d30_43fe_843f_74eab7d3043c
#define _4acc859b_9d30_43fe_843f_74eab7d3043c

#include "Condition.h"

namespace dopamine
{

namespace converterBSON
{

/// @brief Always False Condition
class AlwaysFalse : public Condition
{
public:
    /// @brief Constructor.
    AlwaysFalse();

    /// @brief Destructor.
    virtual ~AlwaysFalse();

    /// @brief Always return false.
    virtual bool operator()(
        odil::Tag const & tag, odil::Element const & element) const;
};

} // namespace converterBSON

} // namespace dopamine

#endif // _4acc859b_9d30_43fe_843f_74eab7d3043c
