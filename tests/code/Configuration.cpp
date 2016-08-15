/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE Configuration
#include <boost/test/unit_test.hpp>

#include <sstream>

#include "dopamine/Configuration.h"
#include "dopamine/Exception.h"

BOOST_AUTO_TEST_CASE(Minimal)
{
    std::stringstream stream;
    stream << "[database]" << "\n";
    stream << "hostname = pacs.example.com" << "\n";
    stream << "dbname = dopamine" << "\n";
    stream << "[dicom]" << "\n";
    stream << "port = 11112" << "\n";

    dopamine::Configuration const configuration(stream);
    BOOST_REQUIRE(configuration.is_valid());
    BOOST_REQUIRE_EQUAL(configuration.get_mongo_host(), "pacs.example.com");
    BOOST_REQUIRE_EQUAL(configuration.get_mongo_port(), 27017);
    BOOST_REQUIRE_EQUAL(configuration.get_database(), "dopamine");
    BOOST_REQUIRE_EQUAL(configuration.get_bulk_database(), "");
    BOOST_REQUIRE_EQUAL(configuration.get_archive_port(), 11112);
    BOOST_REQUIRE_EQUAL(configuration.get_logger_priority(), "WARN");
    BOOST_REQUIRE_EQUAL(configuration.get_logger_destination(), "");
}

BOOST_AUTO_TEST_CASE(Full)
{
    std::stringstream stream;
    stream << "[database]" << "\n";
    stream << "hostname = pacs.example.com" << "\n";
    stream << "port = 1234" << "\n";
    stream << "dbname = dopamine" << "\n";
    stream << "bulk_data = other" << "\n";
    stream << "[dicom]" << "\n";
    stream << "port = 11112" << "\n";
    stream << "[logger]" << "\n";
    stream << "priority = INFO" << "\n";
    stream << "destination = /var/log/dopamine.log" << "\n";

    dopamine::Configuration const configuration(stream);
    BOOST_REQUIRE(configuration.is_valid());
    BOOST_REQUIRE_EQUAL(configuration.get_mongo_host(), "pacs.example.com");
    BOOST_REQUIRE_EQUAL(configuration.get_mongo_port(), 1234);
    BOOST_REQUIRE_EQUAL(configuration.get_database(), "dopamine");
    BOOST_REQUIRE_EQUAL(configuration.get_bulk_database(), "other");
    BOOST_REQUIRE_EQUAL(configuration.get_archive_port(), 11112);
    BOOST_REQUIRE_EQUAL(configuration.get_logger_priority(), "INFO");
    BOOST_REQUIRE_EQUAL(
        configuration.get_logger_destination(), "/var/log/dopamine.log");
}

BOOST_AUTO_TEST_CASE(MissingHost)
{
    std::stringstream stream;
    stream << "[database]" << "\n";
    stream << "dbname = dopamine" << "\n";
    stream << "[dicom]" << "\n";
    stream << "port = 11112" << "\n";

    dopamine::Configuration const configuration(stream);
    BOOST_REQUIRE(!configuration.is_valid());
    BOOST_REQUIRE_THROW(configuration.get_mongo_host(), dopamine::Exception);
    BOOST_REQUIRE_EQUAL(configuration.get_mongo_port(), 27017);
    BOOST_REQUIRE_EQUAL(configuration.get_database(), "dopamine");
    BOOST_REQUIRE_EQUAL(configuration.get_bulk_database(), "");
    BOOST_REQUIRE_EQUAL(configuration.get_archive_port(), 11112);
    BOOST_REQUIRE_EQUAL(configuration.get_logger_priority(), "WARN");
    BOOST_REQUIRE_EQUAL(configuration.get_logger_destination(), "");
}

BOOST_AUTO_TEST_CASE(MissingDatabase)
{
    std::stringstream stream;
    stream << "[database]" << "\n";
    stream << "hostname = pacs.example.com" << "\n";
    stream << "[dicom]" << "\n";
    stream << "port = 11112" << "\n";

    dopamine::Configuration const configuration(stream);
    BOOST_REQUIRE(!configuration.is_valid());
    BOOST_REQUIRE_EQUAL(configuration.get_mongo_host(), "pacs.example.com");
    BOOST_REQUIRE_EQUAL(configuration.get_mongo_port(), 27017);
    BOOST_REQUIRE_THROW(configuration.get_database(), dopamine::Exception);
    BOOST_REQUIRE_EQUAL(configuration.get_bulk_database(), "");
    BOOST_REQUIRE_EQUAL(configuration.get_archive_port(), 11112);
    BOOST_REQUIRE_EQUAL(configuration.get_logger_priority(), "WARN");
    BOOST_REQUIRE_EQUAL(configuration.get_logger_destination(), "");
}

BOOST_AUTO_TEST_CASE(MissingArchivePort)
{
    std::stringstream stream;
    stream << "[database]" << "\n";
    stream << "hostname = pacs.example.com" << "\n";
    stream << "dbname = dopamine" << "\n";
    stream << "[dicom]" << "\n";

    dopamine::Configuration const configuration(stream);
    BOOST_REQUIRE(!configuration.is_valid());
    BOOST_REQUIRE_EQUAL(configuration.get_mongo_host(), "pacs.example.com");
    BOOST_REQUIRE_EQUAL(configuration.get_mongo_port(), 27017);
    BOOST_REQUIRE_EQUAL(configuration.get_database(), "dopamine");
    BOOST_REQUIRE_EQUAL(configuration.get_bulk_database(), "");
    BOOST_REQUIRE_THROW(configuration.get_archive_port(), dopamine::Exception);
    BOOST_REQUIRE_EQUAL(configuration.get_logger_priority(), "WARN");
    BOOST_REQUIRE_EQUAL(configuration.get_logger_destination(), "");
}
