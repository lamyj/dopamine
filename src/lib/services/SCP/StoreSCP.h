/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _57ab17de_94cf_44f4_8311_2a22f7360f34
#define _57ab17de_94cf_44f4_8311_2a22f7360f34

#include <dcmtkpp/message/CStoreRequest.h>

#include "services/SCP/SCP.h"

namespace dopamine
{

namespace services
{
    
/**
 * @brief \class SCP for C-STORE services
 */
class StoreSCP : public SCP
{
public:
    /// @brief Callback called when a request is received.
    typedef std::function<dcmtkpp::Value::Integer(
            dcmtkpp::Association const &,
            dcmtkpp::message::CStoreRequest const &,
            Generator::Pointer)> Callback;

    /// @brief Default constructor.
    StoreSCP();

    /// @brief Constructor with default callback.
    StoreSCP(dcmtkpp::Network * network, dcmtkpp::Association * association);

    /// @brief Constructor.
    StoreSCP(dcmtkpp::Network * network, dcmtkpp::Association * association,
             Callback const & callback);

    /// @brief Destructor.
    virtual ~StoreSCP();

    /// @brief Return the callback.
    Callback const & get_callback() const;

    /// @brief Set the callback.
    void set_callback(Callback const & callback);

    /// @brief Process a C-Store request.
    virtual void operator()(dcmtkpp::message::Message const & message);

private:
    Callback _callback;
    
};

} // namespace services
    
} // namespace dopamine

#endif // _57ab17de_94cf_44f4_8311_2a22f7360f34
