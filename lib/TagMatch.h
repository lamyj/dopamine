#ifndef _a9036539_65b5_4e6b_acc6_ac598bc2275c
#define _a9036539_65b5_4e6b_acc6_ac598bc2275c

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctagkey.h>
#include "Condition.h"

class DcmElement;

class TagMatch : public Condition
{
public :
    typedef boost::shared_ptr<TagMatch> Pointer;
    static Pointer New(DcmTagKey tag);

    TagMatch(DcmTagKey tag);
    virtual bool operator()(DcmElement * element) const;
    DcmTagKey tag;
};

#endif // _a9036539_65b5_4e6b_acc6_ac598bc2275c
