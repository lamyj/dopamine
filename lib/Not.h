#ifndef _542f8cac_93b1_458b_a6a5_6671301a7196
#define _542f8cac_93b1_458b_a6a5_6671301a7196

#include <boost/shared_ptr.hpp>
#include "Condition.h"

class DcmElement;

class Not : public Condition
{
public :
    typedef boost::shared_ptr<Not> Pointer;
    static Pointer New(Condition::Pointer const & condition);

    Not(Condition::Pointer const & condition);
    virtual bool operator()(DcmElement * element) const;
    Condition::Pointer condition;
};

#endif // _542f8cac_93b1_458b_a6a5_6671301a7196
