/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _361310be_d429_4f49_9d0d_19bd01316dff
#define _361310be_d429_4f49_9d0d_19bd01316dff

#include "Condition.h"

class DcmElement;

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief \class IsPrivateTag Condition
 */
class IsPrivateTag : public Condition
{
public:
    typedef boost::shared_ptr<IsPrivateTag> Pointer;
    
    /// Create pointer to new instance of IsPrivateTag
    static Pointer New();

    /// Destroy the instance of IsPrivateTag
    virtual ~IsPrivateTag();

    /**
     * Operator ()
     * @param element: tested element
     * @return true if element is Private, false otherwise
     * @throw ExceptionPACS if element is null
     */
    virtual bool operator()(DcmElement * element)
            const throw(dopamine::ExceptionPACS);
    
protected:

private:
    /// Create an instance of IsPrivateTag
    IsPrivateTag();

};

} // namespace converterBSON

} // namespace dopamine

#endif // _361310be_d429_4f49_9d0d_19bd01316dff
