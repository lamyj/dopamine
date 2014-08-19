#ifndef _8a4c809c_65e2_494d_8c80_8186a778dd92
#define _8a4c809c_65e2_494d_8c80_8186a778dd92

#include <vector>
#include <boost/shared_ptr.hpp>
#include "Condition.h"

class DcmElement;

class And : public Condition
{
public :
    typedef boost::shared_ptr<And> Pointer;
    static Pointer New();

    And();
    virtual bool operator()(DcmElement * element) const throw(research_pacs::ExceptionPACS);
    std::vector<boost::shared_ptr<Condition> > conditions;
};

#endif // _8a4c809c_65e2_494d_8c80_8186a778dd92
