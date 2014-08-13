/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <iostream>

#include "core/ConfigurationPACS.h"
#include "core/DBConnection.h"
#include "core/NetworkPACS.h"

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cerr << "Syntax: " << argv[0] << " <config_file>\n";
        return EXIT_FAILURE;
    }
    
    // Read configuration file
    research_pacs::ConfigurationPACS::get_instance().Parse(argv[1]);
    
    // Create DB connection
    research_pacs::DBConnection::get_instance().connect();
    
    // Create and run Network listener
    research_pacs::NetworkPACS network;
    network.run();
    
    return EXIT_SUCCESS;
}
