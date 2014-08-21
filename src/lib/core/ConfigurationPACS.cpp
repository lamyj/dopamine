/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
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

namespace research_pacs
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
::Parse(std::string const & file)
{
    if ( ! boost::filesystem::exists(file.c_str()))
    {
        throw ExceptionPACS("Trying to parse non-existing file.");
    }
    
    boost::property_tree::ini_parser::read_ini(file, this->_confptree);
    
    // read allowed AETitle
    this->_AETitles.clear();
    if (!this->HasValue("dicom.allowed_peers"))
    {
        throw ExceptionPACS("Missing mandatory node: dicom.allowed_peers");
    }
    std::string value = this->GetValue("dicom.allowed_peers");
    boost::split(this->_AETitles, value, boost::is_any_of(","));
    
    // read list of addresses and ports
    this->_addressPortList.clear();
    std::vector<std::string> templist;
    value = this->GetValue("listAddressPort.allowed");
    boost::split(templist, value, boost::is_any_of(","));
    for (auto aetitle : templist)
    {
        std::string addressport = this->GetValue("listAddressPort", aetitle);
        this->_addressPortList.insert(std::pair<std::string, std::string>(aetitle, addressport));
    }
}

std::string 
ConfigurationPACS
::GetValue(std::string const & key)
{
    if (!this->HasValue(key))
    {
        return "";
    }
    return this->_confptree.get<std::string>(key);
}

std::string 
ConfigurationPACS
::GetValue(std::string const & section, std::string const & key)
{
    return this->GetValue(section + "." + key);
}

bool 
ConfigurationPACS
::HasValue(std::string const & key)
{
    auto child = this->_confptree.get_optional<std::string>( key );
    return child;
}

bool 
ConfigurationPACS
::HasValue(std::string const & section, std::string const & key)
{
    return this->HasValue(section + "." + key);
}

bool 
ConfigurationPACS
::peerInAETitle(std::string const & peer)
{
    // '*' => everybody allowed
    if (std::find(this->_AETitles.begin(), this->_AETitles.end(), "*") 
            != this->_AETitles.end())
    {
        return true;
    }
        
    // search for specific AETitle
    return (std::find(this->_AETitles.begin(), 
                      this->_AETitles.end(), 
                      peer.c_str()) != this->_AETitles.end());
}

bool 
ConfigurationPACS
::peerForAETitle(std::string const & AETitle, std::string & addressPort) const
{
    auto item = this->_addressPortList.find(AETitle);
    
    if (item != this->_addressPortList.end())
    {
        addressPort = item->second;
        return true;
    }
    addressPort = "";
    return false;
}

} // namespace research_pacs
