/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _c086ddd9_5204_4bea_bb03_c4fbec6b16ea
#define _c086ddd9_5204_4bea_bb03_c4fbec6b16ea

#include <map>
#include <vector>

#include <boost/property_tree/ptree.hpp>

namespace research_pacs
{

class ConfigurationPACS
{
public:
    static ConfigurationPACS & get_instance();
    
    static void delete_instance();
    
    virtual ~ConfigurationPACS();
    
    void Parse(std::string const & file);
    
    std::string GetValue(std::string const & key);
    std::string GetValue(std::string const & section, std::string const & key);
    
    bool HasValue(std::string const & key);
    bool HasValue(std::string const & section, std::string const & key);
    
    bool peerInAETitle(std::string const & peer);
    
    bool peerForAETitle(std::string const & AETitle, std::string & addressPort) const;

protected:

private:
    ConfigurationPACS();
    
    static ConfigurationPACS * _instance;
    
    boost::property_tree::ptree _confptree;
    
    std::vector<std::string> _AETitles;
    
    std::map<std::string, std::string> _addressPortList;

    // Purposely not implemented
    ConfigurationPACS(ConfigurationPACS const & other);
    void operator=(ConfigurationPACS const & other);
    
};

} // namespace research_pacs

#endif // _c086ddd9_5204_4bea_bb03_c4fbec6b16ea
