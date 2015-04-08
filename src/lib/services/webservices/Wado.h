/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _0276cd0f_e979_42e0_8b48_4359f8a1c69c
#define _0276cd0f_e979_42e0_8b48_4359f8a1c69c

#include <string>

#include <mongo/client/dbclient.h>

namespace dopamine
{

namespace services
{

std::string const authentication_string = "This server could not verify that \
                                           you are authorized to access the \
                                           document requested. Either you supplied \
                                           the wrong credentials (e.g., bad password), \
                                           or your browser doesn't understand how to \
                                           supply the credentials required.";

class Wado
{
public:
    Wado(std::string const & query,
         std::string const & username);

    virtual ~Wado();

    std::string get_filename() const;
    std::string get_response() const;

protected:
    std::string _query;
    std::string _username;
    std::string _response;
    std::string _filename;

    virtual mongo::BSONObj parse_string() = 0;

    std::string get_dataset(mongo::BSONObj const & object);

private:

};

} // namespace services

} // namespace dopamine

#endif // _0276cd0f_e979_42e0_8b48_4359f8a1c69c
