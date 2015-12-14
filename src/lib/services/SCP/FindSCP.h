/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _c3674b2f_3f18_4264_89db_24dd4aaec99a
#define _c3674b2f_3f18_4264_89db_24dd4aaec99a

#include <dcmtkpp/message/CFindRequest.h>

#include "services/SCP/SCP.h"

namespace dopamine
{

namespace services
{
    
/**
 * @brief \class SCP for C-FIND services
 */
class FindSCP : public SCP
{
public:
    /// @brief Default constructor.
    FindSCP();

    /// @brief Constructor.
    FindSCP(dcmtkpp::Network * network, dcmtkpp::Association * association);

    /// @brief Destructor.
    virtual ~FindSCP();

    /// @brief Process a C-Find request.
    virtual void operator()(dcmtkpp::message::Message const & message);

private:

};

} // namespace services

} // namespace dopamine

#endif // _c3674b2f_3f18_4264_89db_24dd4aaec99a
