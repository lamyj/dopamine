/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE QueryDataSetGenerator
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/message/CFindRequest.h>
#include <odil/registry.h>

#include "dopamine/archive/QueryDataSetGenerator.h"

#include "fixtures/MetaData.h"

struct Fixture: public fixtures::MetaData
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
        odil::message::CFindRequest const request(
            1, odil::registry::PatientRootQueryRetrieveInformationModelFIND,
            odil::message::CFindRequest::Priority::MEDIUM, query);

        odil::AssociationParameters parameters;
        parameters.set_user_identity_to_username(principal);

        dopamine::archive::QueryDataSetGenerator generator(
            this->connection, this->acl, this->database, parameters);

        generator.initialize(request);
        std::vector<odil::DataSet> data_sets;
        while(!generator.done())
        {
            data_sets.push_back(generator.get());
            generator.next();
        }

        std::sort(data_sets.begin(), data_sets.end(), fixtures::MetaData::less);

        return data_sets;
    }
};

BOOST_FIXTURE_TEST_CASE(NotAllowed, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"PATIENT"});
    query.add(odil::registry::PatientID, {"1"});
    query.add(odil::registry::PatientName);

    BOOST_REQUIRE_THROW(this->make_query("store", query), odil::SCP::Exception);
}

BOOST_FIXTURE_TEST_CASE(Patient, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"PATIENT"});
    query.add(odil::registry::PatientID, {"1"});
    query.add(odil::registry::PatientName);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::PatientID)
            == odil::Value::Strings({"1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::PatientName)
            == odil::Value::Strings({"Patient 1"}));
}

BOOST_FIXTURE_TEST_CASE(AllPatients, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"PATIENT"});
    query.add(odil::registry::PatientID);
    query.add(odil::registry::PatientName);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 2);
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::PatientID)
            == odil::Value::Strings({"1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::PatientName)
            == odil::Value::Strings({"Patient 1"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::PatientID)
            == odil::Value::Strings({"2"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::PatientName)
            == odil::Value::Strings({"Patient 2"}));
}

BOOST_FIXTURE_TEST_CASE(AllPatientsRestricted, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"PATIENT"});
    query.add(odil::registry::PatientName, {"*"});
    query.add(odil::registry::PatientID);

    auto const data_sets = this->make_query("restricted_query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::PatientID)
            == odil::Value::Strings({"1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::PatientName)
            == odil::Value::Strings({"Patient 1"}));
}

BOOST_FIXTURE_TEST_CASE(StudyWithPatientRoot, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"STUDY"});
    query.add(odil::registry::PatientID, {"2"});
    query.add(odil::registry::StudyDescription);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 2);
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::StudyInstanceUID)
            == odil::Value::Strings({"2.1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::StudyDescription)
            == odil::Value::Strings({"Study 2.1"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::StudyInstanceUID)
            == odil::Value::Strings({"2.2"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::StudyDescription)
            == odil::Value::Strings({"Study 2.2"}));
}

BOOST_FIXTURE_TEST_CASE(StudyWithStudyRoot, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"STUDY"});
    query.add(odil::registry::StudyDescription, {"Study 2.2"});
    query.add(odil::registry::PatientName);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::PatientID)
            == odil::Value::Strings({"2"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::PatientName)
            == odil::Value::Strings({"Patient 2"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::StudyInstanceUID)
            == odil::Value::Strings({"2.2"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::StudyDescription)
            == odil::Value::Strings({"Study 2.2"}));
}

BOOST_FIXTURE_TEST_CASE(AllStudies, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"STUDY"});
    query.add(odil::registry::StudyDescription);
    query.add(odil::registry::PatientName);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 3);
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::PatientID)
            == odil::Value::Strings({"1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::PatientName)
            == odil::Value::Strings({"Patient 1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::StudyInstanceUID)
            == odil::Value::Strings({"1.1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::StudyDescription)
            == odil::Value::Strings({"Study 1.1"}));

    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::PatientID)
            == odil::Value::Strings({"2"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::PatientName)
            == odil::Value::Strings({"Patient 2"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::StudyInstanceUID)
            == odil::Value::Strings({"2.1"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::StudyDescription)
            == odil::Value::Strings({"Study 2.1"}));

    BOOST_REQUIRE(
        data_sets[2].as_string(odil::registry::PatientID)
            == odil::Value::Strings({"2"}));
    BOOST_REQUIRE(
        data_sets[2].as_string(odil::registry::PatientName)
            == odil::Value::Strings({"Patient 2"}));
    BOOST_REQUIRE(
        data_sets[2].as_string(odil::registry::StudyInstanceUID)
            == odil::Value::Strings({"2.2"}));
    BOOST_REQUIRE(
        data_sets[2].as_string(odil::registry::StudyDescription)
            == odil::Value::Strings({"Study 2.2"}));
}

BOOST_FIXTURE_TEST_CASE(Series, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"SERIES"});
    query.add(odil::registry::PatientID, {"2"});
    query.add(odil::registry::StudyInstanceUID, {"2.1"});
    query.add(odil::registry::Modality);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::SeriesInstanceUID)
            == odil::Value::Strings({"2.1.1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::Modality)
            == odil::Value::Strings({"MR"}));
}

BOOST_FIXTURE_TEST_CASE(AllSeriesInStudy, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"SERIES"});
    query.add(odil::registry::PatientID, {"2"});
    query.add(odil::registry::StudyInstanceUID, {"2.2"});
    query.add(odil::registry::Modality);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 2);
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::SeriesInstanceUID)
            == odil::Value::Strings({"2.2.1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::Modality)
            == odil::Value::Strings({"MR"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::SeriesInstanceUID)
            == odil::Value::Strings({"2.2.2"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::Modality)
            == odil::Value::Strings({"CT"}));
}

BOOST_FIXTURE_TEST_CASE(Instance, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"IMAGE"});
    query.add(odil::registry::PatientID, {"2"});
    query.add(odil::registry::StudyInstanceUID, {"2.1"});
    query.add(odil::registry::SeriesInstanceUID, {"2.1.1"});
    query.add(odil::registry::ImageComments);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::SOPInstanceUID)
            == odil::Value::Strings({"2.1.1.1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::ImageComments)
            == odil::Value::Strings({"Instance 2.1.1.1"}));
}

BOOST_FIXTURE_TEST_CASE(AllInstancesInSeries, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"IMAGE"});
    query.add(odil::registry::PatientID, {"2"});
    query.add(odil::registry::StudyInstanceUID, {"2.2"});
    query.add(odil::registry::SeriesInstanceUID, {"2.2.2"});
    query.add(odil::registry::ImageComments);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 2);
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::SOPInstanceUID)
            == odil::Value::Strings({"2.2.2.1"}));
    BOOST_REQUIRE(
        data_sets[0].as_string(odil::registry::ImageComments)
            == odil::Value::Strings({"Instance 2.2.2.1"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::SOPInstanceUID)
            == odil::Value::Strings({"2.2.2.2"}));
    BOOST_REQUIRE(
        data_sets[1].as_string(odil::registry::ImageComments)
            == odil::Value::Strings({"Instance 2.2.2.2"}));
}

BOOST_FIXTURE_TEST_CASE(NumberOfPatientRelatedStudies, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"PATIENT"});
    query.add(odil::registry::PatientName, {"Patient 2"});
    query.add(odil::registry::NumberOfPatientRelatedStudies);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_int(odil::registry::NumberOfPatientRelatedStudies)
            == odil::Value::Integers({2}));
}

BOOST_FIXTURE_TEST_CASE(NumberOfPatientRelatedSeries, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"PATIENT"});
    query.add(odil::registry::PatientName, {"Patient 2"});
    query.add(odil::registry::NumberOfPatientRelatedSeries);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_int(odil::registry::NumberOfPatientRelatedSeries)
            == odil::Value::Integers({3}));
}

BOOST_FIXTURE_TEST_CASE(NumberOfPatientRelatedInstances, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"PATIENT"});
    query.add(odil::registry::PatientName, {"Patient 2"});
    query.add(odil::registry::NumberOfPatientRelatedInstances);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_int(odil::registry::NumberOfPatientRelatedInstances)
            == odil::Value::Integers({4}));
}

BOOST_FIXTURE_TEST_CASE(NumberOfStudyRelatedSeries, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"STUDY"});
    query.add(odil::registry::StudyInstanceUID, {"2.2"});
    query.add(odil::registry::NumberOfStudyRelatedSeries);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_int(odil::registry::NumberOfStudyRelatedSeries)
            == odil::Value::Integers({2}));
}

BOOST_FIXTURE_TEST_CASE(NumberOfStudyRelatedInstances, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"STUDY"});
    query.add(odil::registry::StudyInstanceUID, {"2.2"});
    query.add(odil::registry::NumberOfStudyRelatedInstances);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_int(odil::registry::NumberOfStudyRelatedInstances)
            == odil::Value::Integers({3}));
}

BOOST_FIXTURE_TEST_CASE(NumberOfSeriesRelatedInstances, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"SERIES"});
    query.add(odil::registry::SeriesInstanceUID, {"2.2.2"});
    query.add(odil::registry::NumberOfSeriesRelatedInstances);

    auto const data_sets = this->make_query("query", query);

    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(
        data_sets[0].as_int(odil::registry::NumberOfSeriesRelatedInstances)
            == odil::Value::Integers({2}));
}

BOOST_FIXTURE_TEST_CASE(ModalitiesInStudy, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"STUDY"});
    query.add(odil::registry::StudyInstanceUID, {"2.2"});
    query.add(odil::registry::ModalitiesInStudy);

    auto const data_sets = this->make_query("query", query);
    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(data_sets[0].has(odil::registry::ModalitiesInStudy));
    auto modalities_in_study = data_sets[0].as_string(
        odil::registry::ModalitiesInStudy);
    std::sort(modalities_in_study.begin(), modalities_in_study.end());
    BOOST_REQUIRE(
        modalities_in_study == odil::Value::Strings({"CT", "MR"}));
}

BOOST_FIXTURE_TEST_CASE(SOPClassesInStudy, Fixture)
{
    odil::DataSet query;
    query.add(odil::registry::QueryRetrieveLevel, {"STUDY"});
    query.add(odil::registry::StudyInstanceUID, {"2.2"});
    query.add(odil::registry::SOPClassesInStudy);

    auto const data_sets = this->make_query("query", query);
    BOOST_REQUIRE_EQUAL(data_sets.size(), 1);
    BOOST_REQUIRE(data_sets[0].has(odil::registry::SOPClassesInStudy));
    auto modalities_in_study = data_sets[0].as_string(
        odil::registry::SOPClassesInStudy);
    std::sort(modalities_in_study.begin(), modalities_in_study.end());
    BOOST_REQUIRE(
        modalities_in_study == odil::Value::Strings({ 
            odil::registry::CTImageStorage, odil::registry::MRImageStorage }));
}

