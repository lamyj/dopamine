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

#include <dcmtkpp/DataSet.h>
#include <dcmtkpp/Element.h>
#include <dcmtkpp/Tag.h>

#include "core/ExceptionPACS.h"

namespace dopamine
{

/**
 * @brief \class Base class condition
 */
class Condition_DEBUG_RLA
{
public:
    typedef boost::shared_ptr<Condition_DEBUG_RLA> Pointer;

    /// Destroy the instance of Condition
    virtual ~Condition_DEBUG_RLA() {}

    /**
     * @brief operator (): function should be implement in derived classes
     * @param tag
     * @param element
     * @return
     */
    virtual bool operator()(dcmtkpp::Tag const & tag,
                            dcmtkpp::Element const & element)
            const throw(dopamine::ExceptionPACS) = 0;

protected:
    /// Create an instance of Condition
    Condition_DEBUG_RLA() {}

private:

};

} // namespace dopamine

#endif // _22e89421_2015_4250_9fe5_63d84409adf7
