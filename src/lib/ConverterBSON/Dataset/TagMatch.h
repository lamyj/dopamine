/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _a9036539_65b5_4e6b_acc6_ac598bc2275c
#define _a9036539_65b5_4e6b_acc6_ac598bc2275c

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctagkey.h>

#include "Condition.h"

class DcmElement;

namespace dopamine
{

/**
 * @brief TagMatch Condition
 */
class TagMatch : public Condition
{
public :
    typedef boost::shared_ptr<TagMatch> Pointer;
    
    /// Create pointer to new instance of TagMatch
    static Pointer New(DcmTagKey tag);
    
    /// Destroy the instance of TagMatch
    virtual ~TagMatch();

    /**
     * Operator ()
     * @param element: tested element
     * @return true if element's tag match with Searched Tag, false otherwise
     * @throw ExceptionPACS if element is null
     */
    virtual bool operator()(DcmElement * element) const throw(dopamine::ExceptionPACS);

protected:
    
private:
    /// Create an instance of TagMatch
    TagMatch(DcmTagKey tag);
    
    /// Searched Tag
    DcmTagKey _tag;

};

} // namespace dopamine

#endif // _a9036539_65b5_4e6b_acc6_ac598bc2275c
