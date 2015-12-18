/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <iostream>

#include <boost/filesystem.hpp>

#include "core/ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "core/NetworkPACS.h"

int main(int argc, char** argv)
{
    std::string const syntax = "dopamine -f CONFIG_FILE";
    if(argc != 3 || std::string(argv[1]) != std::string("-f"))
    {
        std::cerr << "Syntax: " << syntax << "\n";
        return 1;
    }

    // Read configuration file
    dopamine::ConfigurationPACS& configuration =
            dopamine::ConfigurationPACS::get_instance();
    std::string const config_file(argv[2]);
    if(!boost::filesystem::exists(config_file))
    {
        std::cerr << "No such file: '" << config_file << "'\n";
        std::cerr << "Syntax: " << syntax << "\n";
        return 1;
    }
    configuration.parse(config_file);

    // Create and Initialize Logger
    auto const priority =
        dopamine::ConfigurationPACS::get_instance().get_value("logger.priority");
    auto const destination =
        dopamine::ConfigurationPACS::get_instance().get_value("logger.destination");
    auto const path =
        (destination=="file")?
        dopamine::ConfigurationPACS::get_instance().get_value("logger.path"):"";
    dopamine::initialize_logger(priority, destination, path);

    // Create and run Network listener
    dopamine::NetworkPACS::get_instance().run();
    dopamine::NetworkPACS::delete_instance();

    return EXIT_SUCCESS;
}
