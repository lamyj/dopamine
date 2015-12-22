/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
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

#include "dbconnection/MongoDBInformation.h"

namespace dopamine
{

/**
 * @brief \class to create and manage the Configuration
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
    void parse(std::string const & file);
    
    /**
     * Get value of a given key
     * @param key: searched key (like 'Section.key')
     * @return value of the given key, empty string if key does not exist
     */
    std::string get_value(std::string const & key) const;
    
    /**
     * Get value of a given key
     * @param section: searched section
     * @param key: searched key
     * @return value of the given key, empty string if key does not exist
     */
    std::string get_value(std::string const & section,
                          std::string const & key) const;
    
    /**
     * Check if key is in configuration file
     * @param key: searched key
     * @return true if key exist, false otherwise
     */
    bool has_value(std::string const & key) const;
    
    /**
     * Check if key is in configuration file
     * @param section: searched section
     * @param key: searched key
     * @return true if key exist, false otherwise
     */
    bool has_value(std::string const & section, std::string const & key) const;
    
    /**
     * Get network address and port for a given AE Title
     * @param aetitle: searched AE Title (in)
     * @param address_and_port: network address ('address:port') (out)
     * @return true if aetitle is listed in configuration, false otherwise
     */
    bool peer_for_aetitle(std::string const & aetitle,
                          std::string & address_and_port) const;

    void get_database_configuration(MongoDBInformation & db_info,
                                    std::string & hostname,
                                    int & port,
                                    std::vector<std::string> & indexes);

protected:

private:
    /// Create an instance of ConfigurationPACS
    ConfigurationPACS();
    
    /// Unique Instance
    static ConfigurationPACS * _instance;
    
    /// Configuration nodes
    boost::property_tree::ptree _configuration_node;
    
    /// List of known peers (AETitle <=> address)
    std::map<std::string, std::string> _peers;

    // Purposely not implemented
    ConfigurationPACS(ConfigurationPACS const & other);
    void operator=(ConfigurationPACS const & other);
    
};

} // namespace dopamine

#endif // _c086ddd9_5204_4bea_bb03_c4fbec6b16ea
