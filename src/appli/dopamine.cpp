/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "core/ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "core/NetworkPACS.h"

int main(int argc, char** argv)
{
    char* conffile = getenv("DOPAMINE_TEST_CONFIG");
    std::string NetworkConfFILE;
    if (conffile != NULL)
    {
        NetworkConfFILE = std::string(conffile);
    }
    // Read configuration file
    dopamine::ConfigurationPACS& configuration =
            dopamine::ConfigurationPACS::get_instance();
    std::string const localconf = "../../../configuration/dopamine_conf.ini";
    if (NetworkConfFILE != "")
    {
        configuration.parse(NetworkConfFILE);
    }
    else if (boost::filesystem::exists(boost::filesystem::path(localconf)))
    {
        configuration.parse(localconf);
    }
    else
    {
        configuration.parse("/etc/dopamine/dopamine_conf.ini");
    }

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

    return EXIT_SUCCESS;
}
