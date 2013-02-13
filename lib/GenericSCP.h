#ifndef _3d8840e6_2441_4912_9b4a_5020144e85dd
#define _3d8840e6_2441_4912_9b4a_5020144e85dd

#include <string>

#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/dcmnet/scp.h>
#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

/**
 * @brief Generic DICOM SCP.
 */
class GenericSCP : public DcmSCP
{
public :
    GenericSCP(std::string const & db_name,
               std::string const & host="localhost", unsigned int port=27017);
    virtual ~GenericSCP();

protected :
    OFCondition handleIncomingCommand(T_DIMSE_Message *msg,
                                      DcmPresentationContextInfo const & info);
private :
    std::string _db_name;
    mongo::DBClientConnection _connection;
    // mongo::DBClientConnection must be initialized before calling the
    // mongo::GridFS constructor. Since we cannot do this in the initializer
    // list, we need a /pointer/ to mongo::GridFS for late initialization.
    mongo::GridFS* _grid_fs;

    template<T_DIMSE_Command Command>
    OFCondition handleCommand_(T_DIMSE_Message * message,
                               DcmPresentationContextInfo const & info);
};

#include "GenericSCP.txx"

#endif // _3d8840e6_2441_4912_9b4a_5020144e85dd
