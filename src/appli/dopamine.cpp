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
    if (NetworkConfFILE != "")
    {
        dopamine::ConfigurationPACS::get_instance().parse(NetworkConfFILE);
    }
    else if (boost::filesystem::exists(boost::filesystem::path("../../../configuration/dopamine_conf.ini")))
    {
        dopamine::ConfigurationPACS::get_instance().parse("../../../configuration/dopamine_conf.ini");
    }
    else
    {
        dopamine::ConfigurationPACS::get_instance().parse("/etc/dopamine/dopamine_conf.ini");
    }

    // Create and Initialize Logger
    dopamine::initialize_logger
        (
            dopamine::ConfigurationPACS::get_instance().get_value("logger.priority")
        );
    
    // Create and run Network listener
    dopamine::NetworkPACS::get_instance().run();
    
    return EXIT_SUCCESS;
}
