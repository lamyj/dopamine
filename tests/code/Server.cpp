/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE Server
#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <string>
#include <thread>

#include <boost/filesystem.hpp>
#include <log4cpp/Category.hh>
#include <mongo/client/dbclient.h>
#include <odil/Reader.h>

#include "dopamine/Server.h"
#include "dopamine/authentication/AuthenticatorNone.h"

#include "fixtures/SampleData.h"

class Fixture: public fixtures::SampleData
{
public:
    struct Status
    {
        int client;
        std::vector<odil::DataSet> responses;
        dopamine::Server * server;
    };

    uint16_t port;
    uint16_t move_port;
    Status status;

    Fixture()
    : status{ -1, {}, nullptr }
    {
        // Allow un-authenticated access
        this->acl.set_entries({ { "", "*", {} } });

        // Find a random port to use.
        this->port = this->get_port();
    }

    virtual ~Fixture()
    {
        // Nothing to do.
    }

    uint16_t get_port() const
    {
        // Find a random port to run on.
        // WARNING: we have a race condition since we unbind here and rebind
        // in run_server
        boost::asio::io_service service;
        boost::asio::ip::tcp::acceptor acceptor(service);
        boost::asio::ip::tcp::endpoint const endpoint(
            boost::asio::ip::tcp::v4(), 0);
        acceptor.open(endpoint.protocol());
        acceptor.bind(endpoint);
        return acceptor.local_endpoint().port();
    }

    static void run_server(
        mongo::DBClientConnection & connection,
        std::string const & database, uint16_t port, Status & status)
    {
        dopamine::authentication::AuthenticatorNone authenticator;
        dopamine::Server server(
            connection, database, "", port, authenticator);
        status.server = &server;
        server.run();
    }

    static void echo(uint16_t port, Status & status)
    {
        std::string command = "echoscu 127.0.0.1 " + std::to_string(port);
        status.client = system(command.c_str());
    }

    static void find(uint16_t port, Status & status)
    {
        std::string command = "findscu "
            "-S -k QueryRetrieveLevel=STUDY "
            "-k PatientID=2 -k StudyDescription "
            "-q -X "
            "127.0.0.1 "+std::to_string(port);
        status.client = system(command.c_str());

        boost::filesystem::directory_iterator end;
        for(boost::filesystem::directory_iterator it("."); it != end; ++it )
        {
            if(!boost::filesystem::is_regular_file(it->status()))
            {
                continue;
            }
            auto const filename = it->path().stem().string();
            if(filename.substr(0, 3) != "rsp")
            {
                continue;
            }

            std::ifstream stream(it->path().string());
            auto const data_set = odil::Reader::read_file(stream).second;
            status.responses.push_back(data_set);

            boost::filesystem::remove(it->path());
        }

        std::sort(
            status.responses.begin(), status.responses.end(),
            fixtures::SampleData::less);
    }

    static void get(uint16_t port, Status & status)
    {
        std::string command = "getscu "
            "-ll error "
            "-P -k QueryRetrieveLevel=Patient -k PatientID=2 "
            "+B 127.0.0.1 "+std::to_string(port);
        status.client = system(command.c_str());

        boost::filesystem::directory_iterator end;
        for(boost::filesystem::directory_iterator it("."); it != end; ++it )
        {
            if(!boost::filesystem::is_regular_file(it->status()))
            {
                continue;
            }
            auto const filename = it->path().stem().string();
            if(filename.substr(0, 2) != "2.")
            {
                continue;
            }

            std::ifstream stream(it->path().string());
            auto const data_set = odil::Reader::read_file(stream).second;
            status.responses.push_back(data_set);

            boost::filesystem::remove(it->path());
        }

        std::sort(
            status.responses.begin(), status.responses.end(),
            fixtures::SampleData::less);
    }

    static void move(uint16_t port, uint16_t move_port, Status & status)
    {
        std::string command = "movescu "
            "-ll error "
            "-aem CLIENT +P " + std::to_string(move_port) + " "
            + "-P -k QueryRetrieveLevel=Patient -k PatientID=2 "
            + " 127.0.0.1 "+std::to_string(port);
        status.client = system(command.c_str());

        boost::filesystem::directory_iterator end;
        for(boost::filesystem::directory_iterator it("."); it != end; ++it )
        {
            if(!boost::filesystem::is_regular_file(it->status()))
            {
                continue;
            }
            auto const filename = it->path().stem().string();
            if(filename.substr(0, 4) != "MR.2" && filename.substr(0, 4) != "CT.2")
            {
                continue;
            }

            std::ifstream stream(it->path().string());
            auto const data_set = odil::Reader::read_file(stream).second;
            status.responses.push_back(data_set);

            boost::filesystem::remove(it->path());
        }

        std::sort(
            status.responses.begin(), status.responses.end(),
            fixtures::SampleData::less);
    }
};

BOOST_FIXTURE_TEST_CASE(Nothing, Fixture)
{
    std::thread server(
        Fixture::run_server,
        std::ref(this->connection), this->database, this->port,
        std::ref(this->status));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Disable the ERROR message
    log4cpp::Category::getInstance("dopamine").setPriority(log4cpp::Priority::FATAL);

    status.server->shutdown();
    server.join();

    BOOST_REQUIRE(this->status.responses.empty());
}

BOOST_FIXTURE_TEST_CASE(Echo, Fixture)
{
    std::thread server(
        Fixture::run_server,
        std::ref(this->connection), this->database, this->port,
        std::ref(this->status));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread client(Fixture::echo, this->port, std::ref(status));

    // Disable the ERROR message
    log4cpp::Category::getInstance("dopamine").setPriority(log4cpp::Priority::FATAL);

    client.join();
    status.server->shutdown();
    server.join();

    BOOST_REQUIRE_EQUAL(status.client, 0);

    BOOST_REQUIRE(this->status.responses.empty());
}

BOOST_FIXTURE_TEST_CASE(Find, Fixture)
{
    std::thread server(
        Fixture::run_server,
        std::ref(this->connection), this->database, this->port,
        std::ref(this->status));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread client(
        Fixture::find, this->port, std::ref(status));

    // Disable the ERROR message
    log4cpp::Category::getInstance("dopamine").setPriority(log4cpp::Priority::FATAL);

    client.join();
    status.server->shutdown();
    server.join();

    BOOST_REQUIRE_EQUAL(status.client, 0);

    BOOST_REQUIRE_EQUAL(this->status.responses.size(), 2);

    BOOST_REQUIRE(
        status.responses[0].as_string("StudyDescription")
            == odil::Value::Strings{"Study 2.1"});
    BOOST_REQUIRE(
        status.responses[1].as_string("StudyDescription")
            == odil::Value::Strings{"Study 2.2"});
}

BOOST_FIXTURE_TEST_CASE(Move, Fixture)
{
    auto const move_port = this->get_port();

    this->connection.insert(
        this->database+".peers",
        BSON(
            "ae_title" << "CLIENT"
            << "host" << "127.0.0.1"
            << "port" << move_port));

    std::thread server(
        Fixture::run_server,
        std::ref(this->connection), this->database, this->port,
        std::ref(this->status));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread client(
        Fixture::move, this->port, move_port, std::ref(status));

    // Disable the ERROR message
    log4cpp::Category::getInstance("dopamine").setPriority(log4cpp::Priority::FATAL);

    client.join();
    status.server->shutdown();
    server.join();

    BOOST_REQUIRE_EQUAL(status.client, 0);

    BOOST_REQUIRE_EQUAL(this->status.responses.size(), 4);

    BOOST_REQUIRE(
        status.responses[0].as_binary("PixelData")
            == odil::Value::Binary({ { 0x2, 0x1, 0x1, 0x1 } }) );
    BOOST_REQUIRE(
        status.responses[1].as_binary("PixelData")
            == odil::Value::Binary({ { 0x2, 0x2, 0x1, 0x1 } }) );
    BOOST_REQUIRE(
        status.responses[2].as_binary("PixelData")
            == odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x1 } }) );
    BOOST_REQUIRE(
        status.responses[3].as_binary("PixelData")
            == odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x2 } }) );
}

BOOST_FIXTURE_TEST_CASE(Get, Fixture)
{
    std::thread server(
        Fixture::run_server,
        std::ref(this->connection), this->database, this->port,
        std::ref(this->status));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread client(Fixture::get, this->port, std::ref(status));

    // Disable the ERROR message
    log4cpp::Category::getInstance("dopamine").setPriority(log4cpp::Priority::FATAL);

    client.join();
    status.server->shutdown();
    server.join();

    BOOST_REQUIRE_EQUAL(status.client, 0);

    BOOST_REQUIRE_EQUAL(this->status.responses.size(), 4);

    BOOST_REQUIRE(
        status.responses[0].as_binary("PixelData")
            == odil::Value::Binary({ { 0x2, 0x1, 0x1, 0x1 } }) );
    BOOST_REQUIRE(
        status.responses[1].as_binary("PixelData")
            == odil::Value::Binary({ { 0x2, 0x2, 0x1, 0x1 } }) );
    BOOST_REQUIRE(
        status.responses[2].as_binary("PixelData")
            == odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x1 } }) );
    BOOST_REQUIRE(
        status.responses[3].as_binary("PixelData")
            == odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x2 } }) );
}
