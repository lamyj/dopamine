/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _8eb0f4c7_a820_49ad_9035_8ac5d025d133
#define _8eb0f4c7_a820_49ad_9035_8ac5d025d133

#include <boost/shared_ptr.hpp>

#include "core/ExceptionPACS.h"

class DcmElement;

namespace dopamine
{

/**
 * @brief \class Base class condition
 */
class Condition
{
public:
    typedef boost::shared_ptr<Condition> Pointer;
    
    /// Destroy the instance of Condition
    virtual ~Condition() {}
    
    /**
     * Operator (), function should be implement in derived classes
     * @param element: tested element
     * @return true
     * @throw ExceptionPACS if element is null
     */
    virtual bool operator()(DcmElement * element)
            const throw(dopamine::ExceptionPACS) = 0;
    
protected:
    /// Create an instance of Condition
    Condition() {}
    
private:

};

} // namespace dopamine

#endif // _8eb0f4c7_a820_49ad_9035_8ac5d025d133
