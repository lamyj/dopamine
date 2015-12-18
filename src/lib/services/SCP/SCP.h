/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _2a40efaa_eb3c_40f4_a8ba_e614ae1fb9f8
#define _2a40efaa_eb3c_40f4_a8ba_e614ae1fb9f8

#include <dcmtkpp/DcmtkAssociation.h>
#include <dcmtkpp/message/Message.h>
#include <dcmtkpp/Network.h>
#include <dcmtkpp/ServiceRole.h>

#include "services/Generator.h"

namespace dopamine
{

namespace services
{
    
/**
 * @brief \class Base class for all SCP.
 */
class SCP : public dcmtkpp::ServiceRole
{
public:
    /// @brief Create a default Service Class Provider
    ///        with no network and no association.
    SCP();

    /// @brief Create a Service Class Provider with network and association.
    SCP(dcmtkpp::Network * network, dcmtkpp::DcmtkAssociation * association);

    /// @brief Destructor
    virtual ~SCP();

    /// @brief Return the generator.
    Generator::Pointer const get_generator() const;

    /// @brief Set the generator.
    void set_generator(Generator::Pointer const generator);

    /// @brief Process a message.
    virtual void operator()(dcmtkpp::message::Message const & message) =0;

protected:
    Generator::Pointer _generator;

private:
    
};

} // namespace services
    
} // namespace dopamine

#endif // _2a40efaa_eb3c_40f4_a8ba_e614ae1fb9f8
