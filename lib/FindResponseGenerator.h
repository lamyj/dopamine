#ifndef _ee9915a2_a504_4b21_8d43_7938c66c526e
#define _ee9915a2_a504_4b21_8d43_7938c66c526e

#include <set>
#include <string>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmnet/dicom.h>
#include <dcmtk/ofstd/ofcond.h>

#include <mongo/client/dbclient.h>

class FindResponseGenerator
{
public :
    FindResponseGenerator(DcmDataset /*const*/ & query, // DcmDataset is not const-correct
                          mongo::DBClientConnection & connection,
                          std::string const & db_name);
    DIC_US status() const;
    OFCondition next(DcmDataset ** responseIdentifiers);
    void cancel();
private :
    DIC_US _status;
    std::string _query_retrieve_level;
    mongo::auto_ptr<mongo::DBClientCursor> _cursor;
    std::set<mongo::BSONObj> _distinct_items;
};

#endif // _ee9915a2_a504_4b21_8d43_7938c66c526e
