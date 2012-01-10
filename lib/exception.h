#ifndef _6bfdee52_5b48_41e0_8c76_ae579679ec92
#define _6bfdee52_5b48_41e0_8c76_ae579679ec92

#include <stdexcept>

namespace research_pacs
{

class exception : public std::exception
{
public :
    explicit exception(std::string const & message);
    exception(exception const & other);
    virtual ~exception() throw();
    
    virtual char const * what() const throw();
private :
    std::string _message;
};

} // namespace research_pacs

#endif // _6bfdee52_5b48_41e0_8c76_ae579679ec92

