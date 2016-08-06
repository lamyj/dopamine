/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE mongo_query
#include <boost/test/unit_test.hpp>

#include <mongo/bson/bson.h>
#include <odil/DataSet.h>
#include <odil/registry.h>

#include "dopamine/archive/mongo_query.h"

BOOST_AUTO_TEST_CASE(SingleValueString)
{
    mongo::BSONElement const element(BSON("name" << "foo")["name"]);
    mongo::BSONObjBuilder builder;
    dopamine::archive::as_mongo_query<
        dopamine::archive::MatchType::SingleValue>(
            "Field", "LO", element, builder);
    BOOST_REQUIRE_EQUAL(builder.obj()["Field"].String(), "foo");
}

BOOST_AUTO_TEST_CASE(SingleValuePersonName)
{
    mongo::BSONElement const element(
        BSON("name" << BSON(
            "Alphabetic" << "Doe^John" << "Ideographic" << "Other"))["name"]);
    mongo::BSONObjBuilder builder;
    dopamine::archive::as_mongo_query<
        dopamine::archive::MatchType::SingleValue>(
            "Field", "PN", element, builder);
    BOOST_REQUIRE_EQUAL(builder.obj()["Field.Alphabetic"].String(), "Doe^John");
}

BOOST_AUTO_TEST_CASE(SingleValueInt)
{
    mongo::BSONElement const element(BSON("name" << 12345)["name"]);
    mongo::BSONObjBuilder builder;
    dopamine::archive::as_mongo_query<
        dopamine::archive::MatchType::SingleValue>(
            "Field", "US", element, builder);
    BOOST_REQUIRE_EQUAL(builder.obj()["Field"].Int(), 12345);
}

BOOST_AUTO_TEST_CASE(SingleValueReal)
{
    mongo::BSONElement const element(BSON("name" << 123.45)["name"]);
    mongo::BSONObjBuilder builder;
    dopamine::archive::as_mongo_query<
        dopamine::archive::MatchType::SingleValue>(
            "Field", "FL", element, builder);
    BOOST_REQUIRE_EQUAL(builder.obj()["Field"].Double(), 123.45);
}

BOOST_AUTO_TEST_CASE(ListOfUID)
{
    mongo::BSONElement const element(
        BSON("name" << BSON_ARRAY("1.2.3" << "4.5.6"))["name"]);
    mongo::BSONObjBuilder builder;
    dopamine::archive::as_mongo_query<
        dopamine::archive::MatchType::ListOfUID>(
            "Field", "UI", element, builder);
    BOOST_REQUIRE_EQUAL(
        builder.obj(), BSON(
            "Field" << BSON("$in" << BSON_ARRAY("1.2.3" << "4.5.6"))));
}

BOOST_AUTO_TEST_CASE(WildCardOne)
{
    mongo::BSONElement const element(BSON("name" << "A?B")["name"]);
    mongo::BSONObjBuilder builder;
    dopamine::archive::as_mongo_query<dopamine::archive::MatchType::WildCard>(
            "Field", "LO", element, builder);
    BOOST_REQUIRE_EQUAL(builder.obj()["Field"].regex(), "^A.B$");
}

BOOST_AUTO_TEST_CASE(WildCardAny)
{
    mongo::BSONElement const element(BSON("name" << "A*B")["name"]);
    mongo::BSONObjBuilder builder;
    dopamine::archive::as_mongo_query<dopamine::archive::MatchType::WildCard>(
            "Field", "LO", element, builder);
    BOOST_REQUIRE_EQUAL(builder.obj()["Field"].regex(), "^A.*B$");
}

BOOST_AUTO_TEST_CASE(WildCardEscape)
{
    mongo::BSONElement const element(BSON("name" << "A^B.C")["name"]);
    mongo::BSONObjBuilder builder;
    dopamine::archive::as_mongo_query<dopamine::archive::MatchType::WildCard>(
            "Field", "LO", element, builder);
    BOOST_REQUIRE_EQUAL(builder.obj()["Field"].regex(), "^A\\^B\\.C$");
}

BOOST_AUTO_TEST_CASE(Range)
{
    mongo::BSONElement const element(
        BSON("name" << "20160101-20161231")["name"]);
    mongo::BSONObjBuilder builder;
    dopamine::archive::as_mongo_query<
        dopamine::archive::MatchType::Range>(
            "Field", "DA", element, builder);
    BOOST_REQUIRE_EQUAL(
        builder.obj(), BSON("Field" << BSON(
            "$gte" << "20160101" << "$lte" << "20161231")));
}

BOOST_AUTO_TEST_CASE(MultipleValues)
{
    mongo::BSONElement const element(
        BSON("name" << BSON_ARRAY("Foo" << "Bar"))["name"]);
    mongo::BSONObjBuilder builder;
    dopamine::archive::as_mongo_query<
        dopamine::archive::MatchType::MultipleValues>(
            "Field", "LO", element, builder);
    BOOST_REQUIRE_EQUAL(
        builder.obj(), BSON(
            "$or" << BSON_ARRAY(
                BSON("Field" << "Foo") << BSON("Field" << "Bar"))));
}

BOOST_AUTO_TEST_CASE(QueryPatient)
{
    odil::DataSet data_set;
    data_set.add("QueryRetrieveLevel", {"PATIENT"});
    data_set.add("PatientName", {"Doe^John"});
    data_set.add("PatientSex", {"M"});
    data_set.add("PatientBirthDate", {"1930-1940"});
    data_set.add("PatientMotherBirthName", {"Smith^Jane", "Bloggs^Mary"});

    mongo::BSONArrayBuilder terms;
    mongo::BSONObjBuilder fields;
    dopamine::archive::as_mongo_query(data_set, terms, fields);

    BOOST_REQUIRE_EQUAL(
        terms.arr(), BSON_ARRAY(
            BSON(
                std::string(odil::registry::PatientName)+".Value.Alphabetic"
                << "Doe^John")
            << BSON(
                std::string(odil::registry::PatientBirthDate)+".Value"
                << BSON("$gte" << "1930" << "$lte" << "1940"))
            << BSON(std::string(odil::registry::PatientSex)+".Value" << "M")
            << BSON(
                "$or" << BSON_ARRAY(
                    BSON(
                        std::string(
                            odil::registry::PatientMotherBirthName)+".Value.Alphabetic"
                        << "Smith^Jane")
                    << BSON(
                        std::string(
                            odil::registry::PatientMotherBirthName)+".Value.Alphabetic"
                        << "Bloggs^Mary")
                )
            )
    ));

    BOOST_REQUIRE_EQUAL(
        fields.obj(), BSON(
            std::string(odil::registry::PatientName) << 1
            << std::string(odil::registry::PatientBirthDate) << 1
            << std::string(odil::registry::PatientSex) << 1
            << std::string(odil::registry::PatientMotherBirthName) << 1
            << std::string(odil::registry::PatientID) << 1
    ));
}

BOOST_AUTO_TEST_CASE(QueryStudy)
{
    odil::DataSet data_set;
    data_set.add("QueryRetrieveLevel", {"STUDY"});
    data_set.add("PatientID", {"1.2.3.4"});
    data_set.add("StudyDescription");

    mongo::BSONArrayBuilder terms;
    mongo::BSONObjBuilder fields;
    dopamine::archive::as_mongo_query(data_set, terms, fields);

    BOOST_REQUIRE_EQUAL(
        terms.arr(), BSON_ARRAY(
            BSON(std::string(odil::registry::PatientID)+".Value" << "1.2.3.4")
    ));

    BOOST_REQUIRE_EQUAL(
        fields.obj(), BSON(
            std::string(odil::registry::StudyDescription) << 1
            << std::string(odil::registry::PatientID) << 1
            << std::string(odil::registry::StudyInstanceUID) << 1
    ));
}

BOOST_AUTO_TEST_CASE(QuerySeries)
{
    odil::DataSet data_set;
    data_set.add("QueryRetrieveLevel", {"SERIES"});
    data_set.add("PatientID", {"1.2.3.4"});
    data_set.add("StudyInstanceUID", {"5.6.7.8"});
    data_set.add("SeriesDescription");

    mongo::BSONArrayBuilder terms;
    mongo::BSONObjBuilder fields;
    dopamine::archive::as_mongo_query(data_set, terms, fields);

    BOOST_REQUIRE_EQUAL(
        terms.arr(), BSON_ARRAY(
            BSON(std::string(odil::registry::PatientID)+".Value" << "1.2.3.4")
            << BSON(
                std::string(odil::registry::StudyInstanceUID)+".Value"
                << "5.6.7.8")
    ));

    BOOST_REQUIRE_EQUAL(
        fields.obj(), BSON(
            std::string(odil::registry::SeriesDescription) << 1
            << std::string(odil::registry::PatientID) << 1
            << std::string(odil::registry::StudyInstanceUID) << 1
            << std::string(odil::registry::SeriesInstanceUID) << 1
    ));
}

BOOST_AUTO_TEST_CASE(QueryImage)
{
    odil::DataSet data_set;
    data_set.add("QueryRetrieveLevel", {"IMAGE"});
    data_set.add("PatientID", {"1.2.3.4"});
    data_set.add("StudyInstanceUID", {"5.6.7.8"});
    data_set.add("SeriesInstanceUID", {"9.10.11.12"});
    data_set.add("ImageComments");

    mongo::BSONArrayBuilder terms;
    mongo::BSONObjBuilder fields;
    dopamine::archive::as_mongo_query(data_set, terms, fields);

    BOOST_REQUIRE_EQUAL(
        terms.arr(), BSON_ARRAY(
            BSON(std::string(odil::registry::PatientID)+".Value" << "1.2.3.4")
            << BSON(
                std::string(odil::registry::StudyInstanceUID)+".Value"
                << "5.6.7.8")
            << BSON(
                std::string(odil::registry::SeriesInstanceUID)+".Value"
                << "9.10.11.12")
    ));

    BOOST_REQUIRE_EQUAL(
        fields.obj(), BSON(
            std::string(odil::registry::PatientID) << 1
            << std::string(odil::registry::StudyInstanceUID) << 1
            << std::string(odil::registry::SeriesInstanceUID) << 1
            << std::string(odil::registry::ImageComments) << 1
            << std::string(odil::registry::SOPInstanceUID) << 1
    ));
}
