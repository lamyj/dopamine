/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _542f8cac_93b1_458b_a6a5_6671301a7196
#define _542f8cac_93b1_458b_a6a5_6671301a7196

#include "Condition.h"

class DcmElement;

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief Not Condition
 */
class Not : public Condition
{
public :
    typedef boost::shared_ptr<Not> Pointer;
    
    /**
     * Create pointer to new instance of Not
     * @param condition: tested condition
     * @return this pointer
     */
    static Pointer New(Condition::Pointer const & condition);
    
    /// Destroy the instance of Not
    virtual ~Not();

    /**
     * Operator ()
     * @param element: tested element
     * @return true if condition is false, false otherwise
     * @throw ExceptionPACS if element is null
     */
    virtual bool operator()(DcmElement * element) const throw(dopamine::ExceptionPACS);
    
protected:

private:
    /**
     * Create an instance of Not
     * @param condition: tested condition
     */
    Not(Condition::Pointer const & condition);
    
    /// Tested condition
    Condition::Pointer _condition;

};

} // namespace converterBSON

} // namespace dopamine

#endif // _542f8cac_93b1_458b_a6a5_6671301a7196
