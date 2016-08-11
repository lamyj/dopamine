/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE store
#include <boost/test/unit_test.hpp>

#include <odil/DataSet.h>
#include <odil/message/CStoreRequest.h>
#include <odil/message/Response.h>
#include <odil/registry.h>

#include "dopamine/archive/Storage.h"
#include "dopamine/archive/store.h"

#include "fixtures/Authorization.h"

struct Fixture: public fixtures::Authorization
{
    dopamine::archive::Storage storage;
    odil::DataSet data_set;

    Fixture()
    : storage(this->connection, this->database)
    {
        this->data_set.add(odil::registry::PatientName, { "Patient 1" });
        this->data_set.add(odil::registry::PatientID, { "1" });

        this->data_set.add(odil::registry::StudyInstanceUID, {"1.2"});
        this->data_set.add(odil::registry::StudyDescription, { "Study 1" });

        this->data_set.add(odil::registry::SeriesInstanceUID, { "1.2.3" });
        this->data_set.add(odil::registry::Modality, { "MR" });

        this->data_set.add(odil::registry::SOPInstanceUID, { "1.2.3.4" });
        this->data_set.add(
            odil::registry::SOPClassUID, { odil::registry::RawDataStorage });

        this->data_set.add(
            odil::registry::PixelData,
            { odil::Value::Binary::value_type{ 0x1, 0x2, 0x3, 0x4 } },
            odil::VR::OB);
    }

    ~Fixture()
    {
        // Nothing to do.
    }
};


BOOST_FIXTURE_TEST_CASE(Store, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username("store");

    odil::message::CStoreRequest const request(
        1, odil::registry::RawDataStorage,
        data_set.as_string(odil::registry::SOPInstanceUID, 0),
        odil::message::Message::Priority::MEDIUM, this->data_set);

    auto const status = dopamine::archive::store(
        this->acl, parameters, this->storage, request);
    BOOST_REQUIRE_EQUAL(status, odil::message::Response::Success);
}

BOOST_FIXTURE_TEST_CASE(EchoBadUser, Fixture)
{
    odil::AssociationParameters parameters;
    parameters.set_user_identity_to_username("echo");

    odil::message::CStoreRequest const request(
        1, odil::registry::RawDataStorage,
        data_set.as_string(odil::registry::SOPInstanceUID, 0),
        odil::message::Message::Priority::MEDIUM, this->data_set);

    auto const status = dopamine::archive::store(
        this->acl, parameters, this->storage, request);
    BOOST_REQUIRE_EQUAL(status, odil::message::Response::RefusedNotAuthorized);
}
