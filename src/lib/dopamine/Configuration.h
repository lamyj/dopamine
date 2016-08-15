/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _b85c3db0_00b5_4d95_9dd8_222900adecea
#define _b85c3db0_00b5_4d95_9dd8_222900adecea

#include <cstdint>
#include <istream>
#include <memory>
#include <string>

namespace dopamine
{

class Configuration
{
public:
    /// @brief Constructor.
    Configuration(std::istream & stream);

    /// @brief Parse the configuration.
    void parse(std::istream & stream);

    /// @brief Test whether the current configuration is valid.
    bool is_valid() const;

    /// @brief Return the MongoDB host, or throw an exception if none was defined.
    std::string const & get_mongo_host() const;

    /// @brief Return the MongoDB port, default to 27017.
    uint16_t get_mongo_port() const;

    /// @brief Return the main MongoDB database, or throw an exception if none was defined.
    std::string const & get_database() const;

    /// @brief Return the bulk MongoDB database, default to "".
    std::string const & get_bulk_database() const;

    /// @brief Return the port on which the DICOM archive listens, or throw an exception if none was defined.
    uint16_t get_archive_port() const;

    /// @brief Return the logger priority, default to "WARN".
    std::string const & get_logger_priority() const;

    /// @brief Return the logger destination, default to "" (log to console).
    std::string const & get_logger_destination() const;

private:
    std::shared_ptr<std::string> _mongo_host;
    uint16_t _mongo_port;

    // FIXME: mongo auth
    // http://api.mongodb.com/cplusplus/2.6.1/classmongo_1_1_d_b_client_with_commands.html#aef21a401b2151f3f35c77c0b9c7e00d0

    // FIXME: mongo indices

    std::shared_ptr<std::string> _database;
    std::string _bulk_database;

    std::shared_ptr<uint16_t> _archive_port;

    // FIXME: authenticator factory?
    std::shared_ptr<std::string> _authenticator;

    std::string _logger_priority;
    std::string _logger_destination;
};

} // namespace dopamine

#endif // _b85c3db0_00b5_4d95_9dd8_222900adecea
