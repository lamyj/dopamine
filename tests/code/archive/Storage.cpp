/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE Storage
#include <boost/test/unit_test.hpp>

#include <odil/DataSet.h>
#include <odil/registry.h>
#include <odil/uid.h>

#include "dopamine/archive/Storage.h"

#include "fixtures/MongoDB.h"

void print_data_set(odil::DataSet const & data_set)
{
    for(auto const & item: data_set)
    {
        std::cout
            << item.first.get_name() << " "
            << odil::as_string(item.second.vr) << " ";
        if(item.second.is_string())
        {
            for(auto const & value: item.second.as_string())
            {
                std::cout << "'" << value << "', ";
            }
        }
        else
        {
            for(auto const & binary_item: item.second.as_binary())
            {
                std::cout << "{ ";
                for(auto const & value: binary_item)
                {
                    std::cout << "'" << value << "', ";
                }
                std::cout << "}, ";
            }
        }
        std::cout << std::endl;
    }
}

struct Fixture: public fixtures::MongoDB
{
    std::string const bulk_database;

    Fixture()
    : bulk_database(this->database+"-bulk")
    {
        // Nothing to do.
    }

    ~Fixture()
    {
        this->connection.dropDatabase(this->bulk_database);
    }

    odil::DataSet get_data_set()
    {
        odil::DataSet data_set;
        data_set.add(odil::registry::SOPClassUID, {odil::registry::RawDataStorage});
        data_set.add(odil::registry::SOPInstanceUID, {odil::generate_uid()});
        data_set.add(
            odil::registry::PixelData, {
                odil::Value::Binary::value_type{'h', 'e', 'l', 'l', 'o', '!' }
            }, odil::VR::OB);
        return data_set;
    }
};

BOOST_FIXTURE_TEST_CASE(MainContent, Fixture)
{
    dopamine::archive::Storage storage(this->connection, this->database);
    // Make sure we do not store in GridFS
    storage.set_gridfs_limit(1000);

    odil::DataSet const data_set = this->get_data_set();
    storage.store(data_set);

    BOOST_REQUIRE(!this->connection.exists(this->database+".fs.files"));
    BOOST_REQUIRE(!this->connection.exists(this->bulk_database));

    auto const stored = storage.retrieve(
        data_set.as_string(odil::registry::SOPInstanceUID, 0));
    BOOST_REQUIRE(stored == data_set);
}

BOOST_FIXTURE_TEST_CASE(MainGridFS, Fixture)
{
    dopamine::archive::Storage storage(this->connection, this->database);
    // Make sure we store in GridFS
    storage.set_gridfs_limit(1);

    odil::DataSet const data_set = this->get_data_set();
    storage.store(data_set);

    BOOST_REQUIRE(
        !this->connection.findOne(this->database+".fs.files", {}).isEmpty());
    BOOST_REQUIRE(!this->connection.exists(this->bulk_database));

    auto const stored = storage.retrieve(
        data_set.as_string(odil::registry::SOPInstanceUID, 0));
    BOOST_REQUIRE(stored == data_set);
}

BOOST_FIXTURE_TEST_CASE(BulkContent, Fixture)
{
    dopamine::archive::Storage storage(
        this->connection, this->database, this->bulk_database);
    // Make sure we do not store in GridFS
    storage.set_gridfs_limit(1000);

    odil::DataSet const data_set = this->get_data_set();
    storage.store(data_set);

    BOOST_REQUIRE(!this->connection.exists(this->database+".fs.files"));
    BOOST_REQUIRE(!this->connection.exists(this->bulk_database+".fs.files"));
    BOOST_REQUIRE(
        !this->connection.findOne(this->bulk_database+".datasets", {}).isEmpty());

    auto const stored = storage.retrieve(
        data_set.as_string(odil::registry::SOPInstanceUID, 0));
    BOOST_REQUIRE(stored == data_set);
}

BOOST_FIXTURE_TEST_CASE(BulkGridFS, Fixture)
{
    dopamine::archive::Storage storage(
        this->connection, this->database, this->bulk_database);
    // Make sure we store in GridFS
    storage.set_gridfs_limit(1);

    odil::DataSet const data_set = this->get_data_set();
    storage.store(data_set);

    BOOST_REQUIRE(!this->connection.exists(this->database+".fs.files"));
    BOOST_REQUIRE(
        !this->connection.findOne(this->bulk_database+".fs.files", {}).isEmpty());
    BOOST_REQUIRE(!this->connection.exists(this->bulk_database+".datasets"));

    auto const stored = storage.retrieve(
        data_set.as_string(odil::registry::SOPInstanceUID, 0));
    BOOST_REQUIRE(stored == data_set);
}
