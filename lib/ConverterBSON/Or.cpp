#include "Or.h"

#include <vector>
#include <boost/shared_ptr.hpp>
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

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
::operator()(DcmElement * element) const
{
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
