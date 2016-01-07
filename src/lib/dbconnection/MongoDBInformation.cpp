/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "MongoDBInformation.h"

namespace dopamine
{

MongoDBInformation
::MongoDBInformation():
    connection(), _db_name(""), _bulk_data(""), _user(""), _password("")
{
    // Nothing else.
}

MongoDBInformation
::~MongoDBInformation()
{
    // Nothing to do.
}

MongoDBInformation
::MongoDBInformation(MongoDBInformation const & other):
    connection(), _db_name(other.get_db_name()),
    _bulk_data(other.get_bulk_data()), _user(other.get_user()),
    _password(other.get_password())
{
    // Nothing else.
}

MongoDBInformation &
MongoDBInformation
::operator=(MongoDBInformation const & other)
{
    if(this != &other)
    {
        this->set_db_name(other.get_db_name());
        this->set_bulk_data(other.get_bulk_data());
        this->set_user(other.get_user());
        this->set_password(other.get_password());
    }

    return *this;
}

std::string const &
MongoDBInformation
::get_db_name() const
{
    return this->_db_name;
}

void
MongoDBInformation
::set_db_name(std::string const & db_name)
{
    this->_db_name = db_name;
}

std::string const &
MongoDBInformation
::get_bulk_data() const
{
    return this->_bulk_data;
}

void
MongoDBInformation
::set_bulk_data(std::string const & bulk_data)
{
    this->_bulk_data = bulk_data;
}

std::string
MongoDBInformation
::get_user() const
{
    return this->_user;
}

void
MongoDBInformation
::set_user(std::string const & user)
{
    this->_user = user;
}

std::string
MongoDBInformation
::get_password() const
{
    return this->_password;
}

void
MongoDBInformation
::set_password(std::string const & password)
{
    this->_password = password;
}

} // namespace dopamine
