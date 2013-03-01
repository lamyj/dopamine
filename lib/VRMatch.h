#ifndef _01db2e6d_df7c_4b7a_ae0e_e04d2896413b
#define _01db2e6d_df7c_4b7a_ae0e_e04d2896413b

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcvr.h>
#include "Condition.h"

class DcmElement;

class VRMatch : public Condition
{
public :
    typedef boost::shared_ptr<VRMatch> Pointer;
    static Pointer New(DcmEVR vr);

    VRMatch(DcmEVR vr);
    virtual bool operator()(DcmElement * element) const;
    DcmEVR vr;
};

#endif // _01db2e6d_df7c_4b7a_ae0e_e04d2896413b
