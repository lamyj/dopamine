#ifndef _ff8d1604_1410_498f_945e_941630fdd05e
#define _ff8d1604_1410_498f_945e_941630fdd05e

#include <vector>
#include <boost/shared_ptr.hpp>
#include "Condition.h"

class DcmElement;

class Or : public Condition
{
public :
    typedef boost::shared_ptr<Or> Pointer;
    static Pointer New();

    Or();
    virtual bool operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS);
    std::vector<boost::shared_ptr<Condition> > conditions;
};

#endif // _ff8d1604_1410_498f_945e_941630fdd05e
