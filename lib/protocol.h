#ifndef _e6a999e4_4204_43c7_8f33_ce190367901a
#define _e6a999e4_4204_43c7_8f33_ce190367901a

#include <string>
#include <mongo/client/dbclient.h>

class Protocol
{
public :
    Protocol(std::string const & id="", std::string const & name="", std::string const & sponsor="");
    
    std::string const & get_id() const;
    void set_id(std::string const & id);
    
    std::string const & get_name() const;
    void set_name(std::string const & name);
    
    std::string const & get_sponsor() const;
    void set_sponsor(std::string const & name);

    mongo::BSONObj to_bson() const;
    void from_bson(mongo::BSONObj const & object);

private :
    std::string _id;
    std::string _name;
    std::string _sponsor;
};

#endif // _e6a999e4_4204_43c7_8f33_ce190367901a

