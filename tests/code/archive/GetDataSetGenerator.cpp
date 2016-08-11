/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE GetDataSetGenerator
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/message/CGetRequest.h>
#include <odil/registry.h>

#include "dopamine/archive/GetDataSetGenerator.h"

#include "fixtures/SampleData.h"

struct Fixture: public fixtures::SampleData
{
    Fixture()
    {
        // Nothing to do.
    }

    ~Fixture()
    {
        // Nothing to do.
    }

    std::vector<odil::DataSet> make_query(
        std::string const & principal, odil::DataSet const & query)
    {
        odil::message::CGetRequest const request(
            1, odil::registry::PatientRootQueryRetrieveInformationModelGET,
            odil::message::CGetRequest::Priority::MEDIUM, query);

        odil::AssociationParameters parameters;
        parameters.set_user_identity_to_username(principal);

        dopamine::archive::GetDataSetGenerator generator(
            this->connection, this->acl, this->database, "", parameters);

        generator.initialize(request);
        std::vector<odil::DataSet> data_sets;
        while(!generator.done())
        {
            data_sets.push_back(generator.get());
            generator.next();
        }

        std::sort(
            data_sets.begin(), data_sets.end(), fixtures::SampleData::less);

        return data_sets;
    }
};

BOOST_FIXTURE_TEST_CASE(NotAllowed, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"PATIENT"});
    query.add(odil::registry::PatientID, {"1"});
    query.add(odil::registry::PatientName);

    BOOST_REQUIRE_THROW(
        this->make_query("store", query), odil::SCP::Exception);
}

BOOST_FIXTURE_TEST_CASE(Image, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"IMAGE"});
    query.add(odil::registry::PatientID, {"2"});
    query.add(odil::registry::StudyInstanceUID, {"2.2"});
    query.add(odil::registry::SeriesInstanceUID, {"2.2.1"});
    query.add(odil::registry::SOPInstanceUID, {"2.2.1.1"});

    auto const data_sets = this->make_query("retrieve", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);

    BOOST_REQUIRE(
        data_sets[0].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x1, 0x1 } }));
}

BOOST_FIXTURE_TEST_CASE(Series, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"SERIES"});
    query.add(odil::registry::PatientID, {"2"});
    query.add(odil::registry::StudyInstanceUID, {"2.2"});
    query.add(odil::registry::SeriesInstanceUID, {"2.2.2"});

    auto const data_sets = this->make_query("retrieve", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 2);

    BOOST_REQUIRE(
        data_sets[0].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x1 } }));
    BOOST_REQUIRE(
        data_sets[1].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x2 } }));
}

BOOST_FIXTURE_TEST_CASE(Study, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"STUDY"});
    query.add(odil::registry::PatientID, {"2"});
    query.add(odil::registry::StudyInstanceUID, {"2.2"});

    auto const data_sets = this->make_query("retrieve", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 3);

    BOOST_REQUIRE(
        data_sets[0].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x1, 0x1 } }));
    BOOST_REQUIRE(
        data_sets[1].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x1 } }));
    BOOST_REQUIRE(
        data_sets[2].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x2 } }));
}

BOOST_FIXTURE_TEST_CASE(Patient, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"Patient"});
    query.add(odil::registry::PatientID, {"2"});

    auto const data_sets = this->make_query("retrieve", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 4);

    BOOST_REQUIRE(
        data_sets[0].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x1, 0x1, 0x1 } }));
    BOOST_REQUIRE(
        data_sets[1].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x1, 0x1 } }));
    BOOST_REQUIRE(
        data_sets[2].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x1 } }));
    BOOST_REQUIRE(
        data_sets[3].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x2 } }));
}

BOOST_FIXTURE_TEST_CASE(AllPatient, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"Patient"});
    query.add(odil::registry::PatientName, {"*"});

    auto const data_sets = this->make_query("retrieve", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 5);

    BOOST_REQUIRE(
        data_sets[0].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x1, 0x1, 0x1, 0x1 } }));
    BOOST_REQUIRE(
        data_sets[1].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x1, 0x1, 0x1 } }));
    BOOST_REQUIRE(
        data_sets[2].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x1, 0x1 } }));
    BOOST_REQUIRE(
        data_sets[3].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x1 } }));
    BOOST_REQUIRE(
        data_sets[4].as_binary(odil::registry::PixelData) ==
            odil::Value::Binary({ { 0x2, 0x2, 0x2, 0x2 } }));
}

