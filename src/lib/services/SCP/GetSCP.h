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
    /// @brief Callback called when a request is received.
    typedef std::function<dcmtkpp::Value::Integer(
            dcmtkpp::Association const &,
            dcmtkpp::message::CGetRequest const &,
            Generator::Pointer)> Callback;

    /// @brief Default constructor.
    GetSCP();

    /// @brief Constructor with default callback.
    GetSCP(dcmtkpp::Network * network, dcmtkpp::Association * association);

    /// @brief Constructor.
    GetSCP(dcmtkpp::Network * network, dcmtkpp::Association * association,
           Callback const & callback);

    /// @brief Destructor.
    virtual ~GetSCP();

    /// @brief Return the callback.
    Callback const & get_callback() const;

    /// @brief Set the callback.
    void set_callback(Callback const & callback);

    /// @brief Process a C-Get request.
    virtual void operator()(dcmtkpp::message::Message const & message);

private:
    Callback _callback;

};

} // namespace services
    
} // namespace dopamine

#endif // _9514edbc_0abe_4f43_bf55_f064fb974d2e
