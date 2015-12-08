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
    /// @brief Callback called when a request is received.
    typedef std::function<dcmtkpp::Value::Integer(dcmtkpp::Association const &,
                                                  dcmtkpp::message::CMoveRequest const &,
                                                  Generator::Pointer)> Callback;

    /// @brief Default constructor.
    MoveSCP();

    /// @brief Constructor with default callback.
    MoveSCP(dcmtkpp::Network * network, dcmtkpp::Association * association);

    /// @brief Constructor.
    MoveSCP(
        dcmtkpp::Network * network, dcmtkpp::Association * association,
        Callback const & callback);

    /// @brief Destructor.
    virtual ~MoveSCP();

    /// @brief Return the callback.
    Callback const & get_callback() const;

    /// @brief Set the callback.
    void set_callback(Callback const & callback);

    /// @brief Process a C-Move request.
    virtual void operator()(dcmtkpp::message::Message const & message);

private:
    Callback _callback;
    
};

} // namespace services
    
} // namespace dopamine

#endif // _7e2166a1_25b3_48eb_8226_abe9d64ba064
