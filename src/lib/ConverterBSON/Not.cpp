#include "Not.h"

#include <boost/shared_ptr.hpp>
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>

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
::operator()(DcmElement * element) const
{
    return !(*this->condition)(element);
}

