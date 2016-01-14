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
    
ConfigurationPACS * ConfigurationPACS::_instance = NULL;

ConfigurationPACS &
ConfigurationPACS
::get_instance()
{
    if(ConfigurationPACS::_instance == NULL)
    {
        ConfigurationPACS::_instance = new ConfigurationPACS();
    }
    return *ConfigurationPACS::_instance;
}

void
ConfigurationPACS
::delete_instance()
{
    if(ConfigurationPACS::_instance != NULL)
    {
        delete ConfigurationPACS::_instance;
    }
    ConfigurationPACS::_instance = NULL;
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
    
    // read allowed AETitle
    this->_aetitles.clear();
    if (!this->has_value("dicom.allowed_peers"))
    {
        throw ExceptionPACS("Missing mandatory node: dicom.allowed_peers");
    }
    std::string value = this->get_value("dicom.allowed_peers");
    boost::split(this->_aetitles, value, boost::is_any_of(","));
    
    // read list of addresses and ports
    this->_peers.clear();

    auto const peers_it = this->_configuration_node.find("peers");
    if(peers_it != this->_configuration_node.not_found())
    {
        for(auto const & peer: peers_it->second)
        {
            this->_peers.insert({peer.first, peer.second.data()});
        }
    }
}

std::string
ConfigurationPACS
::get_value(std::string const & key)
{
    if (!this->has_value(key))
    {
        return "";
    }
    return this->_configuration_node.get<std::string>(key);
}

std::string
ConfigurationPACS
::get_value(std::string const & section, std::string const & key)
{
    return this->get_value(section + "." + key);
}

bool
ConfigurationPACS
::has_value(std::string const & key)
{
    auto child = this->_configuration_node.get_optional<std::string>( key );
    return bool(child);
}

bool
ConfigurationPACS
::has_value(std::string const & section, std::string const & key)
{
    return this->has_value(section + "." + key);
}

bool
ConfigurationPACS
::peer_in_aetitle(std::string const & peer)
{
    // '*' => everybody allowed
    if (std::find(this->_aetitles.begin(), this->_aetitles.end(), "*")
            != this->_aetitles.end())
    {
        return true;
    }
        
    // search for specific AETitle
    return (std::find(this->_aetitles.begin(),
                      this->_aetitles.end(),
                      peer.c_str()) != this->_aetitles.end());
}

bool
ConfigurationPACS
::peer_for_aetitle(std::string const & aetitle,
                   std::string & address_and_port) const
{
    auto item = this->_peers.find(aetitle);
    
    if (item != this->_peers.end())
    {
        address_and_port = item->second;
        return true;
    }
    address_and_port = "";
    return false;
}

} // namespace dopamine
