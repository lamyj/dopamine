/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "SCP.h"

namespace dopamine
{

namespace services
{

SCP
::SCP()
: dcmtkpp::ServiceRole()
{
    // Nothing else.
}

SCP
::SCP(dcmtkpp::Network * network, dcmtkpp::Association * association)
: dcmtkpp::ServiceRole(network, association)
{
    // Nothing else.
}

SCP
::~SCP()
{
    // Nothing to do.
}

Generator::Pointer const
SCP
::get_generator() const
{
    return this->_generator;
}

void
SCP
::set_generator(Generator::Pointer const generator)
{
    this->_generator = generator;
}

void
SCP
::receive_and_process()
{
    auto const message = this->_receive();
    (*this)(message);
}

} // namespace services
    
} // namespace dopamine
