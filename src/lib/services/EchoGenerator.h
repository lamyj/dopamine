/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _72ce70c6_c708_4d73_a28a_9262d3a9d44b
#define _72ce70c6_c708_4d73_a28a_9262d3a9d44b

#include "services/GeneratorPACS.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class Response Generator for C-ECHO services.
 */
class EchoGenerator : public GeneratorPACS
{
public:
    typedef EchoGenerator Self;
    typedef boost::shared_ptr<Self> Pointer;

    /// Create pointer to new instance of EchoGenerator
    static Pointer New();

    /// Destroy the echo response generator
    virtual ~EchoGenerator();

    virtual dcmtkpp::Value::Integer initialize(dcmtkpp::Association const & association,
                                               dcmtkpp::message::Message const & message);

    virtual dcmtkpp::Value::Integer initialize(mongo::BSONObj const & request);

    virtual dcmtkpp::Value::Integer next();

protected:
    /// Create a default echo response generator
    EchoGenerator();

private:

};

} // namespace services

} // namespace dopamine

#endif // _72ce70c6_c708_4d73_a28a_9262d3a9d44b
