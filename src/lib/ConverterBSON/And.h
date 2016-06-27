/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _8a4c809c_65e2_494d_8c80_8186a778dd92
#define _8a4c809c_65e2_494d_8c80_8186a778dd92

#include <memory>
#include <vector>

#include "Condition.h"

namespace dopamine
{

namespace converterBSON
{

/// @brief Conjunction of several conditions.
class And : public Condition
{
public:
    /// @brief Constructor.
    And();

    /// @brief Destructor.
    virtual ~And();

    /// @brief Test whether all of the stored condition are true.
    virtual bool operator()(
        odil::Tag const & tag, odil::Element const & element) const;

    /// @brief Return the terms of the conjunction (read-only).
    std::vector<std::shared_ptr<Condition>> const & get_terms() const;

    /// @brief Return the terms of the conjunction (read-write).
    std::vector<std::shared_ptr<Condition>> & get_terms();

private:
    /// List of conditions
    std::vector<std::shared_ptr<Condition>> _conditions;
};

} // namespace converterBSON

} // namespace dopamine

#endif // _8a4c809c_65e2_494d_8c80_8186a778dd92
