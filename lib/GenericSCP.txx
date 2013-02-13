#ifndef _ec2cacb1_ef83_45b8_b509_6c24ad1b5130
#define _ec2cacb1_ef83_45b8_b509_6c24ad1b5130

#include "GenericSCP.h"

#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/dcmnet/diutil.h>
#include <dcmtk/dcmnet/scp.h>

template<T_DIMSE_Command Command>
OFCondition
GenericSCP
::handleCommand_(T_DIMSE_Message * message, DcmPresentationContextInfo const & info)
{
    OFString tempStr;
    DCMNET_ERROR("Cannot handle this kind of DIMSE command (0x"
        << std::hex << std::setfill('0') << std::setw(4)
        << OFstatic_cast(unsigned int, message->CommandField) << ")");
    DCMNET_DEBUG(DIMSE_dumpMessage(tempStr, *message, DIMSE_INCOMING));
    return DIMSE_BADCOMMANDTYPE;
}

#endif // _ec2cacb1_ef83_45b8_b509_6c24ad1b5130
