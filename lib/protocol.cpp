#include "protocol.h"

#include <stdexcept>
#include <string>

#include <mongo/client/dbclient.h>

namespace research_pacs
{

Protocol
::Protocol(std::string const & id, std::string const & name, std::string const & sponsor)
{
    this->set_id(id);
    this->set_name(name);
    this->set_sponsor(sponsor);
}

std::string const &
Protocol
::get_id() const
{
    return this->_id;
}

void
Protocol
::set_id(std::string const & id)
{
    this->_id = id;
}

std::string const &
Protocol
::get_name() const
{
    return this->_name;
}

void
Protocol
::set_name(std::string const & name)
{
    this->_name = name;
}

std::string const &
Protocol
::get_sponsor() const
{
    return this->_sponsor;
}

void
Protocol
::set_sponsor(std::string const & sponsor)
{
    this->_sponsor = sponsor;
}

mongo::BSONObj
Protocol
::to_bson() const
{
    mongo::BSONObj object(BSON(
        "id" << this->get_id() << 
        "name" << this->get_name() << 
        "sponsor" << this->get_sponsor()));
    return object;
}

void
Protocol
::from_bson(mongo::BSONObj const & object)
{
    if(!object.hasField("id"))
    {
        throw std::runtime_error("Cannot set Protocol from BSON : no such field \"id\"");
    }
    
    this->set_id(object.getStringField("id"));
    this->set_name(object.getStringField("name"));
    this->set_sponsor(object.getStringField("sponsor"));
}

} // namespace research_pacs