#include "user.h"

#include <stdexcept>
#include <string>

#include <mongo/client/dbclient.h>

namespace research_pacs
{

User
::User(std::string const & id, std::string const & name)
: _id(id), _name(name)
{
    // Nothing else
}

std::string const &
User
::get_id() const
{
    return this->_id;
}

void
User
::set_id(std::string const & id)
{
    this->_id = id;
}

std::string const &
User
::get_name() const
{
    return this->_name;
}

void
User
::set_name(std::string const & name)
{
    this->_name = name;
}

mongo::BSONObj
User
::to_bson() const
{
    mongo::BSONObj object(BSON("id" << this->_id << "name" << this->_name));
    return object;
}

void
User
::from_bson(mongo::BSONObj const & object)
{
    if(!object.hasField("id"))
    {
        throw std::runtime_error("Cannot set User from BSON : no such field \"id\"");
    }
    
    this->_id = object.getStringField("id");
    this->_name = object.getStringField("name");
}

} // namespace research_pacs
