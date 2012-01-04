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

class TestDatabaseFixture
{
public :
    TestDatabaseFixture()
    : database_name(get_database_name()), database(database_name)
    {
    }
    
    ~TestDatabaseFixture()
    {
        mongo::DBClientConnection connection;
        connection.connect("localhost");
        connection.dropDatabase(this->database_name);
    }
    
    std::string database_name;
    Database database;
    
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
    
    static int _dummy;
    
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

int TestDatabaseFixture::_dummy(TestDatabaseFixture::initialize_rng());

BOOST_FIXTURE_TEST_SUITE(TestDatabase, TestDatabaseFixture);

BOOST_AUTO_TEST_CASE(User)
{
    mongo::BSONObj const user = BSON(
        "id" << "radiologist" << "name" << "Ronald Radiologist");
    this->database.insert_user(user);
    
    mongo::auto_ptr<mongo::DBClientCursor> cursor = 
        this->database.query(mongo::Query(), NULL, "users");
        
    BOOST_REQUIRE(cursor->more());
    mongo::BSONObj const item = cursor->next();
    BOOST_REQUIRE_EQUAL(item.getStringField("id"), "radiologist");
    BOOST_REQUIRE_EQUAL(item.getStringField("name"), "Ronald Radiologist");
    BOOST_REQUIRE(!cursor->more());
    
    BOOST_REQUIRE_THROW(this->database.insert_user(user), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Protocol)
{
    mongo::BSONObj const user = BSON(
        "id" << "bpc" << "name" << "Big Pharmaceutical Company");
    this->database.insert_user(user);
    
    mongo::BSONObj const protocol = BSON(
        "id" << "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d" <<
        "name" << "Foobaril, phase 2" <<
        "sponsor" << "bpc");
    this->database.insert_protocol(protocol);
    
    mongo::auto_ptr<mongo::DBClientCursor> cursor = 
        this->database.query(mongo::Query(), NULL, "protocols");
        
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
    BOOST_REQUIRE_THROW(this->database.insert_protocol(invalid_protocol), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Dataset)
{
    mongo::BSONObj const user = BSON(
        "id" << "bpc" << "name" << "Big Pharmaceutical Company");
    this->database.insert_user(user);
    
    mongo::BSONObj const protocol = BSON(
        "id" << "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d" <<
        "name" << "Foobaril, phase 2" <<
        "sponsor" << "bpc");
    this->database.insert_protocol(protocol);
    
    gdcm::Reader reader;
    reader.SetFileName("/home/lamy/src/research_pacs/BRAINIX/2182114/801/00070001");
    reader.Read();
    gdcm::DataSet const & dataset = reader.GetFile().GetDataSet();
    
    {
        gdcm::DataElement const & de = dataset.GetDataElement(gdcm::Tag(0x0010,0x0010));
        gdcm::Attribute<0x0010,0x0010> at;
        at.SetFromDataElement(de);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "BRAINIX");
    }
    
    gdcm::DataSet de_identified = this->database.de_identify(dataset);
    
    {
        gdcm::DataElement const & de = de_identified.GetDataElement(gdcm::Tag(0x0010,0x0010));
        BOOST_REQUIRE(de.IsEmpty());
    }
    
    this->database.set_clinical_trial_informations(de_identified, 
        "bpc", "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d", "Sim^Ho");
    
    {
        gdcm::DataElement const & de = 
            de_identified.GetDataElement(gdcm::Tag(0x0012,0x0010));
        gdcm::Attribute<0x0012,0x0010> at;
        at.SetFromDataElement(de);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "bpc");
    }
    {
        gdcm::DataElement const & de = 
            de_identified.GetDataElement(gdcm::Tag(0x0012,0x0020));
        gdcm::Attribute<0x0012,0x0020> at;
        at.SetFromDataElement(de);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d");
    }
    {
        gdcm::DataElement const & de = 
            de_identified.GetDataElement(gdcm::Tag(0x0012,0x0040));
        gdcm::Attribute<0x0012,0x0040> at;
        at.SetFromDataElement(de);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "Sim^Ho");
    }
    
    this->database.insert_dataset(de_identified);

    mongo::auto_ptr<mongo::DBClientCursor> cursor = this->database.query(
        QUERY("(0012|0020)" << "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d"),
        NULL, "documents");

    BOOST_REQUIRE(cursor->more());
    mongo::BSONObj const item = cursor->next();
    BOOST_REQUIRE_EQUAL(item.getStringField("(0012|0020)"), "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d");
    BOOST_REQUIRE_EQUAL(item.getStringField("(0012|0040)"), "Sim^Ho");
    BOOST_REQUIRE_EQUAL(item.getStringField("(0012|0010)"), "bpc");
    BOOST_REQUIRE(!cursor->more());
}

BOOST_AUTO_TEST_SUITE_END();
