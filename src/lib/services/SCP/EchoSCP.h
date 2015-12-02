/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _fc8d9861_729f_4ef3_9723_9588628a4ee4
#define _fc8d9861_729f_4ef3_9723_9588628a4ee4

#include <dcmtkpp/SCP.h>

namespace dopamine
{

namespace services
{

class EchoSCP : public dcmtkpp::SCP
{
public:
    /// @brief Callback called when a request is received.
    typedef std::function<dcmtkpp::Value::Integer(dcmtkpp::Association const &,
                                                  dcmtkpp::message::CEchoRequest const &)> Callback;

    /// @brief Default constructor.
    EchoSCP();

    /// @brief Constructor with default callback.
    EchoSCP(dcmtkpp::Network * network, dcmtkpp::Association * association);

    /// @brief Constructor.
    EchoSCP(
        dcmtkpp::Network * network, dcmtkpp::Association * association,
        Callback const & callback);

    /// @brief Destructor.
    virtual ~EchoSCP();

    /// @brief Return the callback.
    Callback const & get_callback() const;

    /// @brief Set the callback.
    void set_callback(Callback const & callback);

    /// @brief Process a C-Echo request.
    virtual void operator()(dcmtkpp::message::Message const & message);

private:
    Callback _callback;

};

} // namespace services

} // namespace dopamine

#endif // _fc8d9861_729f_4ef3_9723_9588628a4ee4
