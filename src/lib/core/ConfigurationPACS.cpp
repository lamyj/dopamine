/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "ConfigurationPACS.h"
#include "ExceptionPACS.h"

namespace dopamine
{
    
std::shared_ptr<ConfigurationPACS>
ConfigurationPACS
::_instance = nullptr;

ConfigurationPACS &
ConfigurationPACS
::get_instance()
{
    if(ConfigurationPACS::_instance == NULL)
    {
        ConfigurationPACS::_instance =
            std::shared_ptr<ConfigurationPACS>(new ConfigurationPACS());
    }
    return *ConfigurationPACS::_instance;
}

ConfigurationPACS
::ConfigurationPACS()
{
    // nothing to do
}

ConfigurationPACS
::~ConfigurationPACS()
{
    // nothing to do
}

void
ConfigurationPACS
::parse(std::string const & file)
{
    if ( ! boost::filesystem::exists(file.c_str()))
    {
        throw ExceptionPACS("Trying to parse non-existing file: " + file);
    }
    
    boost::property_tree::ini_parser::read_ini(file, this->_configuration_node);
}

std::string
ConfigurationPACS
::get_value(std::string const & key) const
{
    if (!this->has_value(key))
    {
        return "";
    }
    return this->_configuration_node.get<std::string>(key);
}

std::string
ConfigurationPACS
::get_value(std::string const & section, std::string const & key) const
{
    return this->get_value(section + "." + key);
}

bool
ConfigurationPACS
::has_value(std::string const & key) const
{
    auto child = this->_configuration_node.get_optional<std::string>( key );
    return bool(child);
}

bool
ConfigurationPACS
::has_value(std::string const & section, std::string const & key) const
{
    return this->has_value(section + "." + key);
}

void
ConfigurationPACS
::get_database_configuration(MongoDBInformation & db_info,
                             std::string & hostname,
                             int & port,
                             std::vector<std::string> & indexes)
{
    std::string db_name = this->get_value("database.dbname");
    db_info.set_db_name(db_name);
    db_info.set_bulk_data(this->get_value("database.bulk_data"));
    db_info.set_user(this->get_value("database.user"));
    db_info.set_password(this->get_value("database.password"));
    hostname = this->get_value("database.hostname");
    port = atoi(this->get_value("database.port").c_str());

    if(db_info.get_bulk_data().empty())
    {
        db_info.set_bulk_data(db_name);
    }

    // Get all indexes
    std::string const indexlist = this->get_value("database.indexlist");
    std::vector<std::string> indexeslist;
    boost::split(indexeslist, indexlist, boost::is_any_of(";"));
    indexes = indexeslist;
}

} // namespace dopamine
