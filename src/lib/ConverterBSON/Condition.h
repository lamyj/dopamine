#ifndef _8eb0f4c7_a820_49ad_9035_8ac5d025d133
#define _8eb0f4c7_a820_49ad_9035_8ac5d025d133

#include <boost/shared_ptr.hpp>

#include "core/ExceptionPACS.h"

class DcmElement;

class Condition
{
public :
    typedef boost::shared_ptr<Condition> Pointer;
    virtual bool operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS) =0;
};

#endif // _8eb0f4c7_a820_49ad_9035_8ac5d025d133
