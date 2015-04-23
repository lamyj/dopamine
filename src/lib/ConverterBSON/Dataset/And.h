/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _8a4c809c_65e2_494d_8c80_8186a778dd92
#define _8a4c809c_65e2_494d_8c80_8186a778dd92

#include <vector>

#include "Condition.h"

class DcmElement;

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief And condition
 */
class And : public Condition
{
public :
    typedef boost::shared_ptr<And> Pointer;
    
    /// Create pointer to new instance of And
    static Pointer New();
    
    /// Destroy the instance of And
    virtual ~And();
    
    /**
     * Operator (), test if all conditions are true
     * @param element: tested element
     * @return true if all condition are true, false otherwise
     * @throw ExceptionPACS if element is null
     */
    virtual bool operator()(DcmElement * element) const throw(dopamine::ExceptionPACS);
    
    /**
     * Add a new condition
     * @param condition: condition to insert
     */
    void insert_condition(Condition::Pointer condition);
    
protected:

private:
    /// Create an instance of And
    And();
    
    /// List of conditions
    std::vector<Condition::Pointer> _conditions;

};

} // namespace converterBSON

} // namespace dopamine

#endif // _8a4c809c_65e2_494d_8c80_8186a778dd92
