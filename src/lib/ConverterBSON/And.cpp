#include "And.h"

#include <vector>
#include <boost/shared_ptr.hpp>
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "core/ExceptionPACS.h"

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
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    if (element == NULL)
    {
        throw research_pacs::ExceptionPACS("element is NULL.");
    }
    bool value=true;
    for(std::vector<boost::shared_ptr<Condition> >::const_iterator it=this->conditions.begin();
        it != this->conditions.end(); ++it)
    {
        value = value && (**it)(element);
        if(!value)
        {
            break;
        }
    }

    return value;
}
