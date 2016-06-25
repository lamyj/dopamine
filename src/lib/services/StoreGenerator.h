/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _3c13b299_aaa6_45f4_9386_482e29bc6dc1
#define _3c13b299_aaa6_45f4_9386_482e29bc6dc1

#include "services/GeneratorPACS.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class Response Generator for C-STORE services.
 */
class StoreGenerator : public GeneratorPACS
{
public:
    typedef StoreGenerator Self;
    typedef boost::shared_ptr<Self> Pointer;

    /// Create pointer to new instance of StoreGenerator
    static Pointer New();

    /// Destroy the store response generator
    virtual ~StoreGenerator();

    virtual odil::Value::Integer initialize(
            odil::Association const & association,
            odil::message::Message const & message);

    virtual odil::Value::Integer next();

    odil::Value::Integer initialize(odil::DataSet const & dataset);

    std::string get_peer_ae_title() const;

protected:
    /// Create a default store response generator
    StoreGenerator();

private:
    std::string _peer_ae_title;
};

} // namespace services

} // namespace dopamine

#endif // _3c13b299_aaa6_45f4_9386_482e29bc6dc1
