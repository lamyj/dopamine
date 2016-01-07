/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "core/ConfigurationPACS.h"
#include "dbconnection/MongoDBConnection.h"

/**
 * @brief list_authorization
 * @param connection
 * @param arguments
 * @return
 */
bool list_authorization(dopamine::MongoDBConnection & connection,
                        std::vector<std::string> const & arguments)
{
    // No argument
    if (arguments.size() != 0)
    {
        std::cerr << "Syntax: dopamine-adm -f CONFIG_FILE authorization list\n";
        return false;
    }

    // Get all entries
    mongo::unique_ptr<mongo::DBClientCursor> cursor = connection.get_connection().query(connection.get_db_name() + ".authorization");

    // Print all entries
    while (cursor->more())
    {
        mongo::BSONObj current_bson = cursor->next();

        std::cout << "Principal: " << current_bson.getField("principal_name").String()
                  << " ;    Service: " << current_bson.getField("service").String()
                  << " ;    Dataset: " << current_bson.getField("dataset").toString() << std::endl;
    }

    cursor.release();

    return true;
}

/**
 * @brief add_authorization
 * @param connection
 * @param arguments
 * @return
 */
bool add_authorization(dopamine::MongoDBConnection & connection,
                       std::vector<std::string> const & arguments)
{
    // should have 2 or 3 arguments
    if (arguments.size() != 2 && arguments.size() != 3)
    {
        std::cerr << "Syntax: dopamine-adm -f CONFIG_FILE authorization add <PRINCIPAL> <SERVICE> [DATASET_AS_JSON]\n";
        return false;
    }

    mongo::BSONObjBuilder builder;
    builder << "principal_name" << arguments[0]
            << "principal_type" << ""
            << "service" << arguments[1];
    if (arguments.size() == 3)
    {
        builder << "dataset" << mongo::fromjson(arguments[2]);
    }
    else
    {
        builder << "dataset" << mongo::BSONObj();
    }

    connection.get_connection().insert(connection.get_db_name() + ".authorization", builder.obj());

    return true;
}

/**
 * @brief remove_authorization
 * @param connection
 * @param arguments
 * @return
 */
bool remove_authorization(dopamine::MongoDBConnection & connection,
                          std::vector<std::string> const & arguments)
{
    // should have 2 or 3 arguments
    if (arguments.size() != 2 && arguments.size() != 3)
    {
        std::cerr << "Syntax: dopamine-adm -f CONFIG_FILE authorization remove <PRINCIPAL> <SERVICE> [DATASET_AS_JSON]\n";
        return false;
    }

    mongo::BSONObjBuilder builder;
    builder << "principal_name" << arguments[0]
            << "principal_type" << ""
            << "service" << arguments[1];
    if (arguments.size() == 3)
    {
        builder << "dataset" << mongo::fromjson(arguments[2]);
    }

    connection.get_connection().remove(connection.get_db_name() + ".authorization", builder.obj());

    return true;
}

/**
 * @brief list_application_entities
 * @param connection
 * @param arguments
 * @return
 */
bool list_application_entities(dopamine::MongoDBConnection & connection,
                               std::vector<std::string> const & arguments)
{
    // No argument
    if (arguments.size() != 0)
    {
        std::cerr << "Syntax: dopamine-adm -f CONFIG_FILE application_entities list\n";
        return false;
    }

    // Get all entries
    mongo::unique_ptr<mongo::DBClientCursor> cursor = connection.get_connection().query(connection.get_db_name() + ".application_entities");

    // Print all entries
    while (cursor->more())
    {
        mongo::BSONObj current_bson = cursor->next();

        std::cout << "AE_Title: " << current_bson.getField("ae_title").String()
                  << " ;    Host: " << current_bson.getField("host").String()
                  << " ;    Port: " << (int)current_bson.getField("port").Number() << std::endl;
    }

    cursor.release();

    return true;
}

/**
 * @brief add_application_entities
 * @param connection
 * @param arguments
 * @return
 */
bool add_application_entities(dopamine::MongoDBConnection & connection,
                              std::vector<std::string> const & arguments)
{
    // should have 2 or 3 arguments
    if (arguments.size() != 2 && arguments.size() != 3)
    {
        std::cerr << "Syntax: dopamine-adm -f CONFIG_FILE application_entities add <AE_TITLE> <HOST> <PORT>\n";
        return false;
    }

    mongo::BSONObjBuilder builder;
    builder << "ae_title" << arguments[0]
            << "host" << arguments[1]
            << "port" << atoi(arguments[2].c_str());

    connection.get_connection().insert(connection.get_db_name() + ".application_entities", builder.obj());

    return true;
}

/**
 * @brief remove_application_entities
 * @param connection
 * @param arguments
 * @return
 */
bool remove_application_entities(dopamine::MongoDBConnection & connection,
                                 std::vector<std::string> const & arguments)
{
    // should have 3 arguments
    if (arguments.size() != 3)
    {
        std::cerr << "Syntax: dopamine-adm -f CONFIG_FILE application_entities remove <AE_TITLE> <HOST> <PORT>\n";
        return false;
    }

    mongo::BSONObjBuilder builder;
    builder << "ae_title" << arguments[0]
            << "host" << arguments[1]
            << "port" << atoi(arguments[2].c_str());

    connection.get_connection().remove(connection.get_db_name() + ".application_entities", builder.obj());

    return true;
}


///
/// @brief Main function
///
int main(int argc, char** argv)
{
    std::string const syntax = "dopamine-adm -f CONFIG_FILE command [command_options] [command_arguments]";
    if(argc < 4 || std::string(argv[1]) != std::string("-f"))
    {
        std::cerr << "Syntax: " << syntax << "\n";
        return EXIT_FAILURE;
    }

    // Read configuration file
    dopamine::ConfigurationPACS& configuration =
            dopamine::ConfigurationPACS::get_instance();
    std::string const config_file(argv[2]);
    if(!boost::filesystem::exists(config_file))
    {
        std::cerr << "No such file: '" << config_file << "'\n";
        std::cerr << "Syntax: " << syntax << "\n";
        return EXIT_FAILURE;
    }
    configuration.parse(config_file);

    typedef std::map<std::string, std::function<bool (dopamine::MongoDBConnection & connection, std::vector<std::string> const & arguments)> > command_option;

    std::map<std::string, command_option> commands = {
        { "authorization", { { "list", list_authorization },
                             { "add", add_authorization },
                             { "remove", remove_authorization } } },
        { "application_entities", { { "list", list_application_entities },
                                    { "add", add_application_entities },
                                    { "remove", remove_application_entities } } }
    };

    std::string const command(argv[3]);
    if (commands.find(command) == commands.end())
    {
        std::cerr << "Syntax: " << syntax << "\n";
        return EXIT_FAILURE;
    }

    std::string const cmd_option(argv[4]);
    if (commands[command].find(cmd_option) == commands[command].end())
    {
        std::cerr << "Syntax: " << syntax << "\n";
        return EXIT_FAILURE;
    }

    std::vector<std::string> cmd_parameters;
    for (unsigned int i = 5; i < argc; ++i)
    {
        cmd_parameters.push_back(std::string(argv[i]));
    }


    // Get configuration for Database connection
    dopamine::MongoDBInformation db_information;
    std::string db_host = "";
    int db_port = -1;
    std::vector<std::string> indexeslist;
    dopamine::ConfigurationPACS::get_instance()
            .get_database_configuration(db_information, db_host,
                                        db_port, indexeslist);

    // Create connection with Database
    dopamine::MongoDBConnection connection(db_information, db_host,
                                           db_port, indexeslist);

    // Try to connect
    if (!connection.connect())
    {
        std::cerr << "cannot connect to database\n";
        return EXIT_FAILURE;
    }

    if (!commands[command][cmd_option](connection, cmd_parameters))
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
