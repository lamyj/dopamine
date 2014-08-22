/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "core/ExceptionPACS.h"
#include "Or.h"

Or::Pointer
Or
::New()
{
    return Pointer(new Or());
}

Or
::Or():
    Condition()
{
    // Nothing else
}

Or
::~Or()
{
    // Nothing to do
}

bool
Or
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    if (element == NULL)
    {
        throw research_pacs::ExceptionPACS("element is NULL.");
    }
    
    bool value=false;
    for(auto it=this->_conditions.begin(); it != this->_conditions.end(); ++it)
    {
        value = value || (**it)(element);
        if(value)
        {
            break;
        }
    }

    return value;
}

void 
Or
::insert_condition(Condition::Pointer condition)
{
    this->_conditions.push_back(condition);
}
