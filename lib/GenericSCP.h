#ifndef _3d8840e6_2441_4912_9b4a_5020144e85dd
#define _3d8840e6_2441_4912_9b4a_5020144e85dd

#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/dcmnet/scp.h>

/**
 * @brief Generic DICOM SCP.
 */
class GenericSCP : public DcmSCP
{
public :
    virtual ~GenericSCP();

protected :
    OFCondition handleIncomingCommand(T_DIMSE_Message *msg,
                                      DcmPresentationContextInfo const & info);
private :
    template<T_DIMSE_Command Command>
    OFCondition handleCommand_(T_DIMSE_Message * message,
                               DcmPresentationContextInfo const & info);
};

#include "GenericSCP.txx"

#endif // _3d8840e6_2441_4912_9b4a_5020144e85dd
