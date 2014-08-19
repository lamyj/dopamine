#ifndef _0df310f1_c98d_4b26_a02f_be846908e094
#define _0df310f1_c98d_4b26_a02f_be846908e094

#include "Condition.h"

class DcmElement;

class AlwaysTrue : public Condition
{
public :
    typedef boost::shared_ptr<AlwaysTrue> Pointer;
    static Pointer New();

    virtual bool operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS);
};

#endif // _0df310f1_c98d_4b26_a02f_be846908e094
