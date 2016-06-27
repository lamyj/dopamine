/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _ff8d1604_1410_498f_945e_941630fdd05e
#define _ff8d1604_1410_498f_945e_941630fdd05e

#include <memory>
#include <vector>

#include "Condition.h"

namespace dopamine
{

namespace converterBSON
{

/// @brief Disjunction of several conditions.
class Or : public Condition
{
public:
    /// @brief Constructor.
    Or();

    /// @brief Destructor.
    virtual ~Or();

    /// @brief Test whether any of the stored condition is true.
    virtual bool operator()(
        odil::Tag const & tag, odil::Element const & element) const;
    
    /// @brief Return the terms of the disjunction (read-only).
    std::vector<std::shared_ptr<Condition>> const & get_terms() const;

    /// @brief Return the terms of the disjunction (read-write).
    std::vector<std::shared_ptr<Condition>> & get_terms();
    
private:
    /// List of conditions
    std::vector<std::shared_ptr<Condition>> _conditions;
};

} // namespace converterBSON

} // namespace dopamine

#endif // _ff8d1604_1410_498f_945e_941630fdd05e
