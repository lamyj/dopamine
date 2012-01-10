#include "exception.h"

namespace research_pacs
{

exception
::exception(std::string const & message)
: _message(message)
{
    // Nothing else
}

exception
::exception(exception const & other)
: _message(other._message)
{
    // Nothing else
}

exception
::~exception() throw()
{
    // Nothing to do
}

char const *
exception
::what() const throw()
{
    return this->_message.c_str();
}

} // namespace research_pacs
