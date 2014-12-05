/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
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
    if (boost::filesystem::exists(boost::filesystem::path("../../../configuration/pacs_conf.ini")))
    {
        research_pacs::ConfigurationPACS::get_instance().Parse("../../../configuration/pacs_conf.ini");
    }
    else
    {
        research_pacs::ConfigurationPACS::get_instance().Parse("/etc/research_pacs/pacs_conf.ini");
    }

    // Create and Initialize Logger
    research_pacs::InitializeLogger
        (
            research_pacs::ConfigurationPACS::get_instance().GetValue("logger.priority")
        );
    
    // Get all indexes
    std::vector<research_pacs::DBConnection::DatabaseIndex> indexes;
    std::string indexlist = research_pacs::ConfigurationPACS::get_instance().GetValue("database.indexlist");
    std::vector<std::string> indexlistvect;
    boost::split(indexlistvect, indexlist, boost::is_any_of(","));

    for (auto currentIndex : indexlistvect)
    {
        indexes.push_back
            (
                research_pacs::DBConnection::DatabaseIndex
                    (
                        research_pacs::ConfigurationPACS::get_instance().GetValue(currentIndex, "keys"),
                        research_pacs::ConfigurationPACS::get_instance().GetValue(currentIndex, "unique") == "true" ? true:false,
                        research_pacs::ConfigurationPACS::get_instance().GetValue(currentIndex, "name")
                    )
            );
    }

    // Create and Initialize DB connection
    research_pacs::DBConnection::get_instance().Initialize
        (
            research_pacs::ConfigurationPACS::get_instance().GetValue("database.dbname"),
            research_pacs::ConfigurationPACS::get_instance().GetValue("database.hostname"),
            research_pacs::ConfigurationPACS::get_instance().GetValue("database.port"),
            indexes
        );

    // Connect Database
    research_pacs::DBConnection::get_instance().connect();
    
    // Create and run Network listener
    research_pacs::NetworkPACS::get_instance().run();
    
    return EXIT_SUCCESS;
}
