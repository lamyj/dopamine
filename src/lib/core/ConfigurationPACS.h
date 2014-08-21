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

/**
 * @brief Class to create and manage the Configuration
 *        Configuration is based on INI file
 */
class ConfigurationPACS
{
public:
    /**
     * Create (if not exist) and return an unique instance of ConfigurationPACS
     * @return unique instance of ConfigurationPACS
     */
    static ConfigurationPACS & get_instance();
    
    /**
     * Remove the unique instance of ConfigurationPACS
     */
    static void delete_instance();
    
    /// Destroy the Configuration
    virtual ~ConfigurationPACS();
    
    /**
     * Parse a given configuration file
     * @param file: configuration file path
     */
    void Parse(std::string const & file);
    
    /**
     * Get value of a given key
     * @param key: searched key (like 'Section.key')
     * @return value of the given key, empty string if key does not exist
     */
    std::string GetValue(std::string const & key);
    
    /**
     * Get value of a given key
     * @param section: searched section
     * @param key: searched key
     * @return value of the given key, empty string if key does not exist
     */
    std::string GetValue(std::string const & section, std::string const & key);
    
    /**
     * Check if key is in configuration file
     * @param key: searched key
     * @return true if key exist, false otherwise
     */
    bool HasValue(std::string const & key);
    
    /**
     * Check if key is in configuration file
     * @param section: searched section
     * @param key: searched key
     * @return true if key exist, false otherwise
     */
    bool HasValue(std::string const & section, std::string const & key);
    
    /**
     * Check if a given peer is in allowed AE Title
     * @param peer: searched AE Title
     * @return true if AE Title is allowed, false otherwise
     */
    bool peerInAETitle(std::string const & peer);
    
    /**
     * Get network address and port for a given AE Title
     * @param AETitle: searched AE Title (in)
     * @param addressPort: corresponding network address ('address:port') (out)
     * @return true if AETitle is listed in configuration, false otherwise
     */
    bool peerForAETitle(std::string const & AETitle, std::string & addressPort) const;

protected:

private:
    /// Create an instance of ConfigurationPACS
    ConfigurationPACS();
    
    /// Unique Instance
    static ConfigurationPACS * _instance;
    
    /// Configuration nodes
    boost::property_tree::ptree _confptree;
    
    /// Allowed AE Titles
    std::vector<std::string> _AETitles;
    
    /// List of known network (AETitle <=> address)
    std::map<std::string, std::string> _addressPortList;

    // Purposely not implemented
    ConfigurationPACS(ConfigurationPACS const & other);
    void operator=(ConfigurationPACS const & other);
    
};

} // namespace research_pacs

#endif // _c086ddd9_5204_4bea_bb03_c4fbec6b16ea
