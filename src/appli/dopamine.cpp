/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <cstdlib>
#include <fstream>
#include <string>

#include <boost/filesystem.hpp>
#include <log4cpp/Category.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Priority.hh>
#include <mongo/client/dbclient.h>

#include "dopamine/authentication/factory.h"
#include "dopamine/Configuration.h"
#include "dopamine/Server.h"



int main(int argc, char** argv)
{
    std::string const syntax = "dopamine -f CONFIG_FILE";
    if(argc != 3 || std::string(argv[1]) != std::string("-f"))
    {
        std::cerr << "Syntax: " << syntax << "\n";
        return EXIT_FAILURE;
    }

    // Read configuration file
    std::string const config_path(argv[2]);
    if(!boost::filesystem::exists(config_path))
    {
        std::cerr << "No such file: '" << config_path << "'\n";
        std::cerr << "Syntax: " << syntax << "\n";
        return 1;
    }
    std::ifstream config_stream(config_path);
    dopamine::Configuration const configuration(config_stream);
    if(!configuration.is_valid())
    {
        std::cerr << "Invalid configuration in '" << config_path << "'\n";
        std::cerr << "Syntax: " << syntax << "\n";
        return 1;
    }

    // Initialize Logger
    auto & logger = log4cpp::Category::getInstance("dopamine");
    logger.setPriority(
        log4cpp::Priority::getPriorityValue(
            configuration.get_logger_priority()));

    auto const & destination = configuration.get_logger_destination();
    log4cpp::OstreamAppender * appender;
    if(destination.empty())
    {
        appender = new log4cpp::OstreamAppender("console", &std::cout);
    }
    else
    {
        auto * log_stream = new std::ofstream(destination);
        appender = new log4cpp::OstreamAppender("console", log_stream);
    }
    appender->setLayout(new log4cpp::BasicLayout());

    logger.removeAllAppenders();
    logger.addAppender(appender);

    // Create the MongoDB connection
    mongo::DBClientConnection connection;
    connection.connect(mongo::HostAndPort(
        configuration.get_mongo_host(), configuration.get_mongo_port()));

    // Create and run Network listener
    auto authenticator = dopamine::authentication::factory(
        configuration.get_authentication());
    dopamine::Server server(
        connection,
        configuration.get_database(), configuration.get_bulk_database(),
        configuration.get_archive_port(), *authenticator);
    server.run();

    return EXIT_SUCCESS;
}
