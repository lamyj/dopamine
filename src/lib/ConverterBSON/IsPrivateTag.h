#ifndef _361310be_d429_4f49_9d0d_19bd01316dff
#define _361310be_d429_4f49_9d0d_19bd01316dff

#include <boost/shared_ptr.hpp>
#include "Condition.h"

class DcmElement;

class IsPrivateTag : public Condition
{
public :
    typedef boost::shared_ptr<IsPrivateTag> Pointer;
    static Pointer New();

    IsPrivateTag();
    virtual bool operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS);
};

#endif // _361310be_d429_4f49_9d0d_19bd01316dff
