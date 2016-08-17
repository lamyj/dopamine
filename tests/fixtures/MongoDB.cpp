/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "MongoDB.h"

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sys/time.h>

#include <mongo/client/dbclient.h>

namespace {

class Appender : public mongo::logger::MessageLogDomain::EventAppender {
public:
    Appender() {}

    virtual mongo::Status append(const mongo::logger::MessageLogDomain::EventAppender::Event& event) {
        std::cout
            << event.getSeverity() << " "
            << event.getDate() << ": "
            << event.getMessage() << std::endl;
        return mongo::Status::OK();
    }
};

mongo::client::Options::LogAppenderPtr make_appender() {
    return mongo::client::Options::LogAppenderPtr(new Appender());
}

}  // namespace

namespace fixtures
{

MongoDB
::MongoDB()
: database(this->_generate_database_name())
{
    if(!this->_client_initialized)
    {
        this->_initialize_mongo_client();
        this->_client_initialized = true;
    }
    this->connection.connect("localhost");
}

MongoDB
::~MongoDB()
{
    this->connection.dropDatabase(this->database);
}

unsigned long const
MongoDB
::_seed(MongoDB::_get_seed());

bool MongoDB::_client_initialized(false);

unsigned long
MongoDB
::_get_seed()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    unsigned long const seed(1000000*tv.tv_sec + tv.tv_usec);

    std::srand(seed);
    return seed;
}

std::string
MongoDB
::_generate_database_name() const
{
    std::string database;
    for(int i=0; i<20; ++i)
    {
        database += 'A'+int(std::rand()/float(RAND_MAX)*26.);
    }
    return database;
}

void
MongoDB
::_initialize_mongo_client()
{
    auto const options = mongo::client::Options()
        .setLogAppenderFactory(&make_appender)
        .setMinLoggedSeverity(mongo::logger::LogSeverity::Log());

    auto const status = mongo::client::initialize(options);
    if(!status.isOK())
    {
        throw std::runtime_error(
            "Could not initialize MongoDB client: "+status.toString());
    }
}

} // namespace fixtures
