#include "Or.h"

#include <vector>
#include <boost/shared_ptr.hpp>
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "core/ExceptionPACS.h"

Or::Pointer
Or
::New()
{
    return Pointer(new Or());
}

Or
::Or()
{
    // Nothing else
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
    for(std::vector<boost::shared_ptr<Condition> >::const_iterator it=this->conditions.begin();
        it != this->conditions.end(); ++it)
    {
        value = value || (**it)(element);
        if(value)
        {
            break;
        }
    }

    return value;
}
