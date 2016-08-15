/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/Configuration.h"

#include <cstdint>
#include <istream>
#include <map>
#include <memory>
#include <string>

#include <boost/property_tree/ini_parser.hpp>
#include <log4cpp/Priority.hh>

#include "dopamine/Exception.h"

namespace
{

template<typename T>
void
set(
    boost::property_tree::ptree const & tree, std::string const & path,
    T & value)
{
    auto const item = tree.get_optional<T>(path);
    if(item)
    {
        value = item.get();
    }
}

template<typename T>
void
set(
    boost::property_tree::ptree const & tree, std::string const & path,
    std::shared_ptr<T> & value)
{
    auto const item = tree.get_optional<T>(path);
    if(item)
    {
        value = std::make_shared<T>(item.get());
    }
}

}

namespace dopamine
{

Configuration
::Configuration(std::istream & stream)
{
    this->parse(stream);
}

void
Configuration
::parse(std::istream & stream)
{
    this->_mongo_host = nullptr;
    this->_mongo_port = 27017;
    this->_database = nullptr;
    this->_bulk_database = "";
    this->_archive_port = nullptr;
    this->_authentication.clear();
    this->_logger_priority = "WARN";
    this->_logger_destination = "";

    boost::property_tree::ptree tree;
    boost::property_tree::ini_parser::read_ini(stream, tree);

    set(tree, "database.hostname", this->_mongo_host);
    set(tree, "database.port", this->_mongo_port);
    set(tree, "database.dbname", this->_database);
    set(tree, "database.bulk_data", this->_bulk_database);
    set(tree, "dicom.port", this->_archive_port);
    set(tree, "logger.priority", this->_logger_priority);
    set(tree, "logger.destination", this->_logger_destination);

    auto const & authentication = tree.get_child_optional("authentication");
    if(authentication)
    {
        for(auto const & item: authentication.get())
        {
            this->_authentication[item.first] = item.second.data();
        }
    }
}

bool
Configuration
::is_valid() const
{
    return (
        this->_mongo_host && this->_database && this->_archive_port
        && !this->_authentication.empty());
}

std::string const &
Configuration
::get_mongo_host() const
{
    if(this->_mongo_host)
    {
        return *this->_mongo_host;
    }
    else
    {
        throw Exception("No MongoDB host");
    }
}

uint16_t
Configuration
::get_mongo_port() const
{
    return this->_mongo_port;
}

std::string const &
Configuration
::get_database() const
{
    if(this->_database)
    {
        return *this->_database;
    }
    else
    {
        throw Exception("No main database");
    }
}

std::string const &
Configuration
::get_bulk_database() const
{
    return this->_bulk_database;
}

uint16_t
Configuration
::get_archive_port() const
{
    if(this->_archive_port)
    {
        return *this->_archive_port;
    }
    else
    {
        throw Exception("No archive port");
    }
}

std::map<std::string, std::string> const &
Configuration
::get_authentication() const
{
    return this->_authentication;
}

std::string const &
Configuration
::get_logger_priority() const
{
    return this->_logger_priority;
}

std::string const &
Configuration
::get_logger_destination() const
{
    return this->_logger_destination;
}

} // namespace dopamine
