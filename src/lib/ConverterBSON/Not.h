/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _542f8cac_93b1_458b_a6a5_6671301a7196
#define _542f8cac_93b1_458b_a6a5_6671301a7196

#include <memory>
#include "Condition.h"

namespace dopamine
{

namespace converterBSON
{

/// @brief Negate a Condition
class Not : public Condition
{
public:
    /// @brief Constructor.
    Not(std::shared_ptr<Condition> const & condition);

    /// @brief Destructor.
    virtual ~Not();

    /// @brief Return the opposite of stored condition
    virtual bool operator()(
        odil::Tag const & tag, odil::Element const & element) const;

private:
    std::shared_ptr<Condition> _condition;

};

} // namespace converterBSON

} // namespace dopamine

#endif // _542f8cac_93b1_458b_a6a5_6671301a7196
