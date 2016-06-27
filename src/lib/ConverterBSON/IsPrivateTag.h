/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _361310be_d429_4f49_9d0d_19bd01316dff
#define _361310be_d429_4f49_9d0d_19bd01316dff

#include "Condition.h"

namespace dopamine
{

namespace converterBSON
{

/// @brief Condition matching private tags.
class IsPrivateTag : public Condition
{
public:
    /// @brief Constructor.
    IsPrivateTag();

    /// @brief Destructor.
    virtual ~IsPrivateTag();

    /// @brief Test whether tag is private.
    virtual bool operator()(
        odil::Tag const & tag, odil::Element const & element) const;
};

} // namespace converterBSON

} // namespace dopamine

#endif // _361310be_d429_4f49_9d0d_19bd01316dff
