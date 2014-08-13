/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "ConfigurationPACS.h"

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
    boost::property_tree::ini_parser::read_ini(file, this->_confptree);
    
    // read allowed AETitle
    this->_AETitles.clear();
    std::string value = this->GetValue("dicom.allowed_peers");
    boost::split(this->_AETitles, value, boost::is_any_of(","));
}

std::string 
ConfigurationPACS
::GetValue(std::string const & key)
{
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

} // namespace research_pacs
