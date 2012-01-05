#ifndef _4bca776c_a256_4cdb_9c45_400d5c2ec0f1
#define _4bca776c_a256_4cdb_9c45_400d5c2ec0f1

#include <string>

#include <gdcmDataSet.h>
#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

class Database
{
public :
    Database(std::string const & db_name, std::string const & host="localhost", unsigned int port=27017);
    ~Database();
    
    void insert_user(mongo::BSONObj const & user);
    void insert_protocol(mongo::BSONObj const & protocol);
    //void insert_file(std::string const & filename);
    void insert_dataset(gdcm::DataSet const & dataset);
    
    mongo::auto_ptr<mongo::DBClientCursor> query(
        mongo::Query const & query, mongo::BSONObj* fields=NULL, 
        std::string const & ns="documents");
    //void remove();
    
    gdcm::DataSet de_identify(gdcm::DataSet const & dataset) const;
    void set_clinical_trial_informations(gdcm::DataSet & dataset, std::string const & sponsor, std::string const & protocol, std::string const &subject);

    void get_file(std::string const sop_instance_uid, std::ostream & stream) const;

private :
    std::string _db_name;
    mongo::DBClientConnection _connection;
    // mongo::DBClientConnection must be initialized before calling the
    // mongo::GridFS constructor. Since we cannot do this in the initializer
    // list, we need a /pointer/ to mongo::GridFS for late initialization.
    mongo::GridFS* _grid_fs;
};

#endif // _4bca776c_a256_4cdb_9c45_400d5c2ec0f1
