/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _9514edbc_0abe_4f43_bf55_f064fb974d2e
#define _9514edbc_0abe_4f43_bf55_f064fb974d2e

#include <dcmtkpp/message/CGetRequest.h>

#include "services/SCP/SCP.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class SCP for C-GET services
 */
class GetSCP : public SCP
{
public:
    /// @brief Default constructor.
    GetSCP();

    /// @brief Constructor.
    GetSCP(dcmtkpp::Network * network, dcmtkpp::DcmtkAssociation * association);

    /// @brief Destructor.
    virtual ~GetSCP();

    /// @brief Process a C-Get request.
    virtual void operator()(dcmtkpp::message::Message const & message);

private:

};

} // namespace services
    
} // namespace dopamine

#endif // _9514edbc_0abe_4f43_bf55_f064fb974d2e
