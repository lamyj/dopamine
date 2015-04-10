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

class Wado : public Webservices
{
public:
    Wado(std::string const & pathinfo,
         std::string const & querystring,
         std::string const & username);

    virtual ~Wado();

    std::string get_filename() const;

protected:
    std::string _filename;

    std::string get_dataset(mongo::BSONObj const & object);

private:

};

} // namespace services

} // namespace dopamine

#endif // _0276cd0f_e979_42e0_8b48_4359f8a1c69c
