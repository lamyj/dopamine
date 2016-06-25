/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _22e89421_2015_4250_9fe5_63d84409adf7
#define _22e89421_2015_4250_9fe5_63d84409adf7

#include <boost/shared_ptr.hpp>

#include <odil/Element.h>
#include <odil/Tag.h>

#include "core/ExceptionPACS.h"

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
     * @brief operator (): function should be implement in derived classes
     * @param tag
     * @param element
     * @return
     */
    virtual bool operator()(odil::Tag const & tag,
                            odil::Element const & element)
            const throw(dopamine::ExceptionPACS) = 0;

protected:
    /// Create an instance of Condition
    Condition() {}

private:

};

} // namespace dopamine

#endif // _22e89421_2015_4250_9fe5_63d84409adf7
