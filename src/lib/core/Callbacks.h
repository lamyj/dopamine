/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _3a8bf4e4_8c36_4d1a_b1d0_d61caacbad07
#define _3a8bf4e4_8c36_4d1a_b1d0_d61caacbad07

#include <dcmtkpp/Association.h>
#include <dcmtkpp/message/CEchoRequest.h>
#include <dcmtkpp/message/CFindRequest.h>
#include <dcmtkpp/message/CGetRequest.h>
#include <dcmtkpp/message/CMoveRequest.h>
#include <dcmtkpp/message/CStoreRequest.h>
#include <dcmtkpp/Value.h>

#include "services/Generator.h"

namespace dopamine
{

dcmtkpp::Value::Integer echo(dcmtkpp::Association const & association,
                             dcmtkpp::message::CEchoRequest const & request,
                             services::Generator::Pointer generator);

dcmtkpp::Value::Integer find(dcmtkpp::Association const & association,
                             dcmtkpp::message::CFindRequest const & request,
                             services::Generator::Pointer generator);

dcmtkpp::Value::Integer get(dcmtkpp::Association const & association,
                            dcmtkpp::message::CGetRequest const & request,
                            services::Generator::Pointer generator);

dcmtkpp::Value::Integer move(dcmtkpp::Association const & association,
                             dcmtkpp::message::CMoveRequest const & request,
                             services::Generator::Pointer generator);

dcmtkpp::Value::Integer store(dcmtkpp::Association const & association,
                              dcmtkpp::message::CStoreRequest const & request,
                              services::Generator::Pointer generator);

} // namespace dopamine

#endif // _3a8bf4e4_8c36_4d1a_b1d0_d61caacbad07
