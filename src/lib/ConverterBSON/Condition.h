/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _22e89421_2015_4250_9fe5_63d84409adf7
#define _22e89421_2015_4250_9fe5_63d84409adf7

#include <odil/DataSet.h>
#include <odil/Element.h>
#include <odil/Tag.h>

namespace dopamine
{

///@brief Base class for all conversion conditions.
class Condition
{
public:
    /// @brief Constructor.
    Condition();

    /// @brief Destructor.
    virtual ~Condition();

    /// @brief Test whether the tag and element fulfill the condition
    virtual bool operator()(
        odil::Tag const & tag, odil::Element const & element) const =0;
};

} // namespace dopamine

#endif // _22e89421_2015_4250_9fe5_63d84409adf7
