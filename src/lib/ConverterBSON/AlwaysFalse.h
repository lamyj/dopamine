/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _4acc859b_9d30_43fe_843f_74eab7d3043c
#define _4acc859b_9d30_43fe_843f_74eab7d3043c

#include "Condition.h"

class DcmElement;

/**
 * @brief Always False Condition
 */
class AlwaysFalse : public Condition
{
public :
    typedef boost::shared_ptr<AlwaysFalse> Pointer;
    
    /// Create pointer to new instance of AlwaysFalse
    static Pointer New();
    
    /// Destroy the instance of AlwaysFalse
    virtual ~AlwaysFalse();

    /**
     * Operator ()
     * @param element: tested element
     * @return false
     */
    virtual bool operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS);
    
protected:

private:
    /// Create an instance of AlwaysFalse
    AlwaysFalse();

};

#endif // _4acc859b_9d30_43fe_843f_74eab7d3043c

