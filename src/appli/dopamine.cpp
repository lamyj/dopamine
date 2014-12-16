/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include "core/ConfigurationPACS.h"
#include "core/DBConnection.h"
#include "core/LoggerPACS.h"
#include "core/NetworkPACS.h"

int main(int argc, char** argv)
{
    // Read configuration file
    if (boost::filesystem::exists(boost::filesystem::path("../../../configuration/dopamine_conf.ini")))
    {
        dopamine::ConfigurationPACS::get_instance().Parse("../../../configuration/dopamine_conf.ini");
    }
    else
    {
        dopamine::ConfigurationPACS::get_instance().Parse("/etc/dopamine/dopamine_conf.ini");
    }

    // Create and Initialize Logger
    dopamine::InitializeLogger
        (
            dopamine::ConfigurationPACS::get_instance().GetValue("logger.priority")
        );
    
    // Get all indexes
    std::string indexlist = dopamine::ConfigurationPACS::get_instance().GetValue("database.indexlist");
    std::vector<std::string> indexlistvect;
    boost::split(indexlistvect, indexlist, boost::is_any_of(";"));

    // Create and Initialize DB connection
    dopamine::DBConnection::get_instance().Initialize
        (
            dopamine::ConfigurationPACS::get_instance().GetValue("database.dbname"),
            dopamine::ConfigurationPACS::get_instance().GetValue("database.hostname"),
            dopamine::ConfigurationPACS::get_instance().GetValue("database.port"),
            indexlistvect
        );

    // Connect Database
    dopamine::DBConnection::get_instance().connect();
    
    // Create and run Network listener
    dopamine::NetworkPACS::get_instance().run();
    
    return EXIT_SUCCESS;
}
