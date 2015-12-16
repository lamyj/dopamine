/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _7e2166a1_25b3_48eb_8226_abe9d64ba064
#define _7e2166a1_25b3_48eb_8226_abe9d64ba064

#include <dcmtkpp/message/CMoveRequest.h>

#include "services/SCP/SCP.h"

namespace dopamine
{

namespace services
{
    
/**
 * @brief \class SCP for C-MOVE services
 */
class MoveSCP : public SCP
{
public:
    /// @brief Default constructor.
    MoveSCP();

    /// @brief Constructor.
    MoveSCP(dcmtkpp::Network * network, dcmtkpp::DcmtkAssociation * association);

    /// @brief Destructor.
    virtual ~MoveSCP();

    /// @brief Process a C-Move request.
    virtual void operator()(dcmtkpp::message::Message const & message);

private:
    
};

} // namespace services
    
} // namespace dopamine

#endif // _7e2166a1_25b3_48eb_8226_abe9d64ba064
