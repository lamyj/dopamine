/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _ff8d1604_1410_498f_945e_941630fdd05e
#define _ff8d1604_1410_498f_945e_941630fdd05e

#include <vector>

#include "Condition.h"

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief \class Or condition
 */
class Or : public Condition_DEBUG_RLA
{
public:
    typedef boost::shared_ptr<Or> Pointer;
    
    /// Create pointer to new instance of Or
    static Pointer New();
    
    /// Destroy the instance of Or
    virtual ~Or();

    /**
     * Operator (), test if one condition is true
     * @param element: tested element
     * @return true if one condition is true, false otherwise
     * @throw ExceptionPACS if element is null
     */
    virtual bool operator()(dcmtkpp::Tag const & tag,
                            dcmtkpp::Element const & element)
            const throw(dopamine::ExceptionPACS);
    
    /**
     * Add a new condition
     * @param condition: condition to insert
     */
    void insert_condition(Condition_DEBUG_RLA::Pointer condition);
    
protected:

private:
    /// Create an instance of Or
    Or();

    /// List of conditions
    std::vector<Condition_DEBUG_RLA::Pointer> _conditions;

};

} // namespace converterBSON

} // namespace dopamine

#endif // _ff8d1604_1410_498f_945e_941630fdd05e
