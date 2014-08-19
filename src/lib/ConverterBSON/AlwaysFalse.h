#ifndef _4acc859b_9d30_43fe_843f_74eab7d3043c
#define _4acc859b_9d30_43fe_843f_74eab7d3043c

#include "Condition.h"

class DcmElement;

class AlwaysFalse : public Condition
{
public :
    typedef boost::shared_ptr<AlwaysFalse> Pointer;
    static Pointer New();

    virtual bool operator()(DcmElement * element) const;
};

#endif // _4acc859b_9d30_43fe_843f_74eab7d3043c

