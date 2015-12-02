/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _f0b69ac6_7d52_401d_a2f3_d5d3f7d69376
#define _f0b69ac6_7d52_401d_a2f3_d5d3f7d69376

#include <map>
#include <memory>

#include <dcmtkpp/SCP.h>
#include <dcmtkpp/ServiceRole.h>
#include <dcmtkpp/Value.h>

namespace dopamine
{

class SCPDispatcher: public dcmtkpp::ServiceRole
{
public:
    /// @brief Create a default dispatcher with no network and no association.
    SCPDispatcher();

    /// @brief Create a dispatcher with network and association.
    SCPDispatcher(dcmtkpp::Network * network,
                  dcmtkpp::Association * association);

    /// @brief Destructor.
    ~SCPDispatcher();

    bool has_scp(dcmtkpp::Value::Integer command) const;

    dcmtkpp::SCP const & get_scp(dcmtkpp::Value::Integer command) const;
    dcmtkpp::SCP & get_scp(dcmtkpp::Value::Integer command);

    template<typename TSCP>
    void set_scp(dcmtkpp::Value::Integer command, TSCP const & scp);

    bool dispatch();
private:
    typedef std::shared_ptr<dcmtkpp::SCP> SCPPointer;
    std::map<dcmtkpp::Value::Integer, SCPPointer> _providers;
};

}

#include "SCPDispatcher.txx"

#endif // _f0b69ac6_7d52_401d_a2f3_d5d3f7d69376
