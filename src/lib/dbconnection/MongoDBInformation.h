/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _7c421f94_e5ec_4a15_a72e_8cf272151945
#define _7c421f94_e5ec_4a15_a72e_8cf272151945

#include <string>

#include <mongo/client/dbclient.h>

namespace dopamine
{

class MongoDBInformation
{
public:
    MongoDBInformation();

    ~MongoDBInformation();

    MongoDBInformation(MongoDBInformation const & other);

    MongoDBInformation& operator=(MongoDBInformation const & other);

    std::string const & get_db_name() const;

    void set_db_name(std::string const & db_name);

    std::string const & get_bulk_data() const;

    void set_bulk_data(std::string const & bulk_data);

    std::string get_user() const;

    void set_user(std::string const & user);

    std::string get_password() const;

    void set_password(std::string const & password);

    mongo::DBClientConnection connection;

private:
    std::string _db_name;

    std::string _bulk_data;

    std::string _user;

    std::string _password;

};

} // namespace dopamine

#endif // _7c421f94_e5ec_4a15_a72e_8cf272151945
