/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _0276cd0f_e979_42e0_8b48_4359f8a1c69c
#define _0276cd0f_e979_42e0_8b48_4359f8a1c69c

#include "Webservices.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class The Wado class
 */
class Wado : public Webservices
{
public:
    /**
     * @brief Create an instance of Wado
     * @param pathinfo
     * @param querystring
     * @param username
     */
    Wado(std::string const & pathinfo,
         std::string const & querystring);

    /// Destroy the instance of Wado
    virtual ~Wado();

protected:

private:

};

} // namespace services

} // namespace dopamine

#endif // _0276cd0f_e979_42e0_8b48_4359f8a1c69c
