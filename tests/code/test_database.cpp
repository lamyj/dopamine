#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestDatabase
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <sstream>

#include <gdcmAttribute.h>
#include <gdcmDataSet.h>
#include <gdcmReader.h>
#include <mongo/client/dbclient.h>

#include "database.h"
#include "exception.h"
#include "user.h"

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
        this->_database = new research_pacs::Database(_database_name);
    }
    
    ~TestDatabaseFixture()
    {
        delete this->_database;

        mongo::DBClientConnection connection;
        connection.connect("localhost");
        connection.dropDatabase(this->_database_name);
    }

    research_pacs::Database & get_database()
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
    research_pacs::Database * _database;
    
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
    research_pacs::User const user("radiologist", "Ronald Radiologist");
    this->get_database().insert_user(user);
    
    std::vector<research_pacs::User> const users = this->get_database().query_users(mongo::Query());
        
    BOOST_REQUIRE_EQUAL(users.size(), 1);
    BOOST_REQUIRE_EQUAL(users[0].get_id(), "radiologist");
    BOOST_REQUIRE_EQUAL(users[0].get_name(), "Ronald Radiologist");
    
    BOOST_REQUIRE_THROW(this->get_database().insert_user(user), research_pacs::exception);
}

BOOST_AUTO_TEST_CASE(Protocol)
{
    research_pacs::User const user("bpc", "Big Pharmaceutical Company");
    this->get_database().insert_user(user);
    
    research_pacs::Protocol const protocol("6dfd7305-10ac-4c90-8c05-e48f2f2fd88d", 
        "Foobaril, phase 2", "bpc");
    this->get_database().insert_protocol(protocol);
    
    std::vector<research_pacs::Protocol> const protocols = this->get_database().query_protocols(mongo::Query());
        
    BOOST_REQUIRE_EQUAL(protocols.size(), 1);
    BOOST_REQUIRE_EQUAL(protocols[0].get_id(), "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d");
    BOOST_REQUIRE_EQUAL(protocols[0].get_name(), "Foobaril, phase 2");
    BOOST_REQUIRE_EQUAL(protocols[0].get_sponsor(), "bpc");
    
    research_pacs::Protocol const invalid_protocol("6dfd7305-10ac-4c90-8c05-e48f2f2fd88d",
        "Foobaril, phase 2", "unknown");
    BOOST_REQUIRE_THROW(this->get_database().insert_protocol(invalid_protocol),
        research_pacs::exception);
}

BOOST_AUTO_TEST_CASE(DeIdentify)
{
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
}

BOOST_AUTO_TEST_CASE(ClinicalTrialInformations)
{
    gdcm::Reader reader;
    std::string const filename(this->static_data.temp_dir+"/BRAINIX/2182114/801/00070001");
    reader.SetFileName(filename.c_str());
    reader.Read();
    gdcm::DataSet dataset = reader.GetFile().GetDataSet();
    
    research_pacs::User const sponsor("bpc", "Big Pharmaceutical Company");
    research_pacs::Protocol const protocol("6dfd7305-10ac-4c90-8c05-e48f2f2fd88d",
        "Foobaril, phase 2", "bpc");
    
    // Neither sponsor nor protocol in DB
    BOOST_REQUIRE_THROW(
        this->get_database().set_clinical_trial_informations(dataset,
            sponsor, protocol, "Sim^Ho"), research_pacs::exception);
    
    this->get_database().insert_user(sponsor);
    
    // Protocol not in DB
    BOOST_REQUIRE_THROW(
        this->get_database().set_clinical_trial_informations(dataset,
            sponsor, protocol, "Sim^Ho"), research_pacs::exception);
    
    this->get_database().insert_protocol(protocol);
    
    this->get_database().set_clinical_trial_informations(dataset,
        sponsor, protocol, "Sim^Ho");
        
    {
        gdcm::Attribute<0x0012,0x0010> at;
        at.Set(dataset);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "bpc");
    }
    {
        gdcm::Attribute<0x0012,0x0020> at;
        at.Set(dataset);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "6dfd7305-10ac-4c90-8c05-e48f2f2fd88d");
    }
    {
        gdcm::Attribute<0x0012,0x0040> at;
        at.Set(dataset);
        BOOST_REQUIRE_EQUAL(at.GetValue().Trim(), "Sim^Ho");
    }
}

BOOST_AUTO_TEST_CASE(Dataset)
{   
    gdcm::Reader reader;
    std::string const filename(this->static_data.temp_dir+"/BRAINIX/2182114/801/00070001");
    reader.SetFileName(filename.c_str());
    reader.Read();
    gdcm::DataSet dataset = reader.GetFile().GetDataSet();
    
    // Neither Clinical Trial Sponsor Name nor Clinical Trial Protocol ID nor
    // Clinical Trial Subject ID
    BOOST_REQUIRE_THROW(this->get_database().insert_dataset(dataset),
        research_pacs::exception);
    
    {
        gdcm::Attribute<0x0012,0x0010> attribute;
        attribute.SetValue("bpc");
        dataset.Insert(attribute.GetAsDataElement());
    }
    
    // Clinical Trial Sponsor Name not in DB, neither Clinical Trial Protocol ID nor
    // Clinical Trial Subject ID
    BOOST_REQUIRE_THROW(this->get_database().insert_dataset(dataset),
        research_pacs::exception);
    
    research_pacs::User const sponsor("bpc", "Big Pharmaceutical Company");
    this->get_database().insert_user(sponsor);
    
    // Neither Clinical Trial Protocol ID nor Clinical Trial Subject ID
    BOOST_REQUIRE_THROW(this->get_database().insert_dataset(dataset),
        research_pacs::exception);
    
    {
        gdcm::Attribute<0x0012,0x0020> attribute;
        attribute.SetValue("6dfd7305-10ac-4c90-8c05-e48f2f2fd88d");
        dataset.Insert(attribute.GetAsDataElement());
    }
    
    // Clinical Trial Protocol ID not in DB, no Clinical Trial Subject ID
    BOOST_REQUIRE_THROW(this->get_database().insert_dataset(dataset),
        research_pacs::exception);

    research_pacs::Protocol const protocol("6dfd7305-10ac-4c90-8c05-e48f2f2fd88d",
        "Foobaril, phase 2", "bpc");
    this->get_database().insert_protocol(protocol);
    
    // No Clinical Trial Subject ID
    BOOST_REQUIRE_THROW(this->get_database().insert_dataset(dataset),
        research_pacs::exception);

    {
        gdcm::Attribute<0x0012,0x0040> attribute;
        attribute.SetValue("Sim^Ho");
        dataset.Insert(attribute.GetAsDataElement());
    }

    this->get_database().insert_dataset(dataset);

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

BOOST_AUTO_TEST_CASE(File)
{
    research_pacs::User const sponsor("bpc", "Big Pharmaceutical Company");
    this->get_database().insert_user(sponsor);

    research_pacs::Protocol const protocol("6dfd7305-10ac-4c90-8c05-e48f2f2fd88d",
        "Foobaril, phase 2", "bpc");
    this->get_database().insert_protocol(protocol);
    
    std::string const & sop_instance_uid = 
        this->get_database().insert_file("tests/data/brainix.tgz", sponsor, protocol, "Sim^Ho");
    
    std::vector<std::string> fields;
    fields.push_back("(0008|0018)");
    fields.push_back("original_mime_type");
    mongo::auto_ptr<mongo::DBClientCursor> cursor = this->get_database().query_documents(
        QUERY("(0008|0018)" << sop_instance_uid), fields);

    BOOST_REQUIRE(cursor->more());
    mongo::BSONObj const item = cursor->next();
    BOOST_REQUIRE_EQUAL(item.getStringField("(0008|0018)"), sop_instance_uid);
    BOOST_REQUIRE_EQUAL(item.getStringField("original_mime_type"), "application/x-gzip");
    BOOST_REQUIRE(!cursor->more());
    
    std::string data;
    {
        std::ostringstream stream;
        this->get_database().get_file(sop_instance_uid, stream);
        data = stream.str();
        
        BOOST_REQUIRE_EQUAL(data.size(), 23708991);
    }
    
    char* expected = new char[23708991];
    {
        std::ifstream stream("tests/data/brainix.tgz");
        stream.read(expected, 23708991);
    }
    BOOST_REQUIRE(std::equal(data.begin(), data.end(), expected));
    delete[] expected;
}

BOOST_AUTO_TEST_SUITE_END();
