#ifndef _65404e35_21b0_4c47_8ceb_83838851a43e
#define _65404e35_21b0_4c47_8ceb_83838851a43e

#include <string>
#include <mongo/client/dbclient.h>

namespace research_pacs
{

class User
{
public :
    User(std::string const & id="", std::string const & name="");
    
    std::string const & get_id() const;
    void set_id(std::string const & id);
    
    std::string const & get_name() const;
    void set_name(std::string const & name);

    mongo::BSONObj to_bson() const;
    void from_bson(mongo::BSONObj const & object);

private :
    std::string _id;
    std::string _name;
};

} // namespace research_pacs

#endif // _65404e35_21b0_4c47_8ceb_83838851a43e

