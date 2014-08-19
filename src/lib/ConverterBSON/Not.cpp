#include "Not.h"

#include <boost/shared_ptr.hpp>
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

#include "core/ExceptionPACS.h"

Not::Pointer
Not
::New(Condition::Pointer const & condition)
{
    return Pointer(new Not(condition));
}

Not
::Not(Condition::Pointer const & condition)
: condition(condition)
{
    // Nothing else
}

bool
Not
::operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS)
{
    if (element == NULL)
    {
        throw research_pacs::ExceptionPACS("element is NULL.");
    }
    return !(*this->condition)(element);
}

