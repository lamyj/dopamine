#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestDatabase
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>

#include <gdcmAttribute.h>
#include <gdcmDataSet.h>
#include <gdcmReader.h>
#include <mongo/client/dbclient.h>
#include "database.h"

struct StaticData
{
    StaticData()
    {
        char tmpl[] = "/tmp/XXXXXX";
        this->temp_dir = mkdtemp(tmpl);

        std::string const command("tar zx -C "+this->temp_dir+" -f tests/data/brainix.tgz");
        system(command.c_str());
    }
    ~StaticData()
    {
        std::string const command("rm -rf "+this->temp_dir);
        system(command.c_str());
    }
    std::string temp_dir;
};

class TestDatabaseFixture
{
public :
    static StaticData const static_data;

    TestDatabaseFixture()
    {
        this->_database_name = get_database_name();
        this->_database = new Database(_database_name);
    }
    
    ~TestDatabaseFixture()
    {
        delete this->_database;

        mongo::DBClientConnection connection;
        connection.connect("localhost");
        connection.dropDatabase(this->_database_name);
    }

    Database & get_database()
    {
        return *(this->_database);
    }
    
    
private :
    class random_char_generator
    {
    public:
        random_char_generator(std::string const & range = "abcdefghijklmnopqrstuvwxyz")
        : _range(range)
        {
        }

        char operator()() const
        {
            return this->_range[std::rand()%this->_range.size()];
        }
    private:
        std::string _range;
    };
    
    static int const _dummy;
    std::string _database_name;
    Database * _database;
    
    static std::string get_database_name(int length=8)
    {
        std::string result;
        result.reserve(length);
        std::generate_n(std::back_inserter(result), length, random_char_generator());
        return result;
    }
    
    static int initialize_rng()
    {
        std::srand(time(NULL));
        return 0;
    }
};

int const TestDatabaseFixture::_dummy(TestDatabaseFixture::initialize_rng());
StaticData const TestDatabaseFixture::static_data = StaticData();

BOOST_FIXTURE_TEST_SUITE(TestDatabase, TestDatabaseFixture);

BOOST_AUTO_TEST_CASE(User)
{
    mongo::BSONObj const user = BSON(
        "id" << "radiologist" << "name" << "Ronald Radiologist");
    this->get_database().insert_user(user);
    
    mongo::auto_ptr<mongo::DBClientCursor> cursor = 
        this->get_database().query_users(mongo::Query());
        
    BOOST_REQUIRE(cursor->more());
    mongo::BSONObj const item = cursor->next();
    BOOST_REQUIRE_EQUAL(item.getStringField("id"), "radiologist");
    BOOST_REQUIRE_EQUAL(item.getStringField("name"), "Ronald Radiologist");
    BOOST_REQUIRE(!cursor->more());
    
    BOOST_REQUIRE_THROW(this->get_database().insert_user(user), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Protocol)
{
    mongo::BSONObj const user = BSON(
        "id" << "bpc" << "name" << "Big Pharmaceutical Company");
    this->get_database().insert_user(user);
    
    mongo::BSONObj const protocol = BSON(
        "id" << "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d" <<
        "name" << "Foobaril, phase 2" <<
        "sponsor" << "bpc");
    this->get_database().insert_protocol(protocol);
    
    mongo::auto_ptr<mongo::DBClientCursor> cursor = 
        this->get_database().query_protocols(mongo::Query());
        
    BOOST_REQUIRE(cursor->more());
    mongo::BSONObj const item = cursor->next();
    BOOST_REQUIRE_EQUAL(item.getStringField("id"), "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d");
    BOOST_REQUIRE_EQUAL(item.getStringField("name"), "Foobaril, phase 2");
    BOOST_REQUIRE_EQUAL(item.getStringField("sponsor"), "bpc");
    BOOST_REQUIRE(!cursor->more());
    
    mongo::BSONObj const invalid_protocol = BSON(
        "id" << "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d" <<
        "name" << "Foobaril, phase 2" <<
        "sponsor" << "unknown");
    BOOST_REQUIRE_THROW(this->get_database().insert_protocol(invalid_protocol), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Dataset)
{
    mongo::BSONObj const user = BSON(
        "id" << "bpc" << "name" << "Big Pharmaceutical Company");
    this->get_database().insert_user(user);

    mongo::BSONObj const protocol = BSON(
        "id" << "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d" <<
        "name" << "Foobaril, phase 2" <<
        "sponsor" << "bpc");
    this->get_database().insert_protocol(protocol);

    gdcm::Reader reader;
    std::string const filename(this->static_data.temp_dir+"/BRAINIX/2182114/801/00070001");
    reader.SetFileName(filename.c_str());
    reader.Read();
    gdcm::DataSet const & dataset = reader.GetFile().GetDataSet();

    {
        gdcm::Attribute<0x0010,0x0010> at;
        at.Set(dataset);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "BRAINIX");
    }

    gdcm::DataSet de_identified = this->get_database().de_identify(dataset);
    BOOST_REQUIRE(!de_identified.FindDataElement(gdcm::Tag(0x0010,0x0010)));

    this->get_database().set_clinical_trial_informations(de_identified,
        "bpc", "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d", "Sim^Ho");

    {
        gdcm::Attribute<0x0012,0x0010> at;
        at.Set(de_identified);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "bpc");
    }
    {
        gdcm::Attribute<0x0012,0x0020> at;
        at.Set(de_identified);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d");
    }
    {
        gdcm::Attribute<0x0012,0x0040> at;
        at.Set(de_identified);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "Sim^Ho");
    }

    this->get_database().insert_dataset(de_identified);

    std::vector<std::string> fields;
    fields.push_back("(0012|0010)");
    fields.push_back("(0012|0020)");
    fields.push_back("(0012|0040)");
    mongo::auto_ptr<mongo::DBClientCursor> cursor = this->get_database().query_documents(
        QUERY("(0012|0020)" << "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d"), fields);

    BOOST_REQUIRE(cursor->more());
    mongo::BSONObj const item = cursor->next();
    BOOST_REQUIRE_EQUAL(item.getStringField("(0012|0020)"), "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d");
    BOOST_REQUIRE_EQUAL(item.getStringField("(0012|0040)"), "Sim^Ho");
    BOOST_REQUIRE_EQUAL(item.getStringField("(0012|0010)"), "bpc");
    BOOST_REQUIRE(!cursor->more());
}

BOOST_AUTO_TEST_SUITE_END();
