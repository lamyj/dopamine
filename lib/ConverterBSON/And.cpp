#include "And.h"

#include <vector>
#include <boost/shared_ptr.hpp>
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

And::Pointer
And
::New()
{
    return Pointer(new And());
}

And
::And()
{
    // Nothing else
}

bool
And
::operator()(DcmElement * element) const
{
    bool value=true;
    for(std::vector<boost::shared_ptr<Condition> >::const_iterator it=this->conditions.begin();
        it != this->conditions.end(); ++it)
    {
        value = value && (**it)(element);
        if(value)
        {
            break;
        }
    }

    return value;
}
