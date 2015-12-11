/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleGeneratorPACS
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/Association.h>
#include <dcmtkpp/message/CEchoRequest.h>
#include <dcmtkpp/message/Response.h>
#include <dcmtkpp/registry.h>

#include "services/GeneratorPACS.h"
#include "ServicesTestClass.h"

// GeneratorPACS is abstract, use inherited class
class GeneratorPACSTest : public dopamine::services::GeneratorPACS
{
public:
    GeneratorPACSTest(): dopamine::services::GeneratorPACS()
    {
        // Nothing else.
    }

    virtual ~GeneratorPACSTest()
    {
        // Nothing to do.
    }

    virtual dcmtkpp::Value::Integer next()
    {
        return dcmtkpp::message::Response::Success;
    }

    void create_cursor()
    {
        this->initialize(mongo::BSONObj());
        mongo::BSONObj const fields();
        mongo::Query const query();
        this->_cursor = this->_connection.query(this->_db_name + ".datasets");
    }

    template<typename T>
    void test_get_match_type(std::string const & vr,
                             std::vector<T> values,
                             Match::Type expected_result)
    {
        mongo::BSONObjBuilder builder;
        for (auto value : values)
        {
            builder << "key" << value;
        }
        auto const match_type = _get_match_type(vr,
                                                builder.obj().getField("key"));
        BOOST_REQUIRE(match_type == expected_result);
    }
};

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    auto generator = new GeneratorPACSTest();
    BOOST_REQUIRE(generator != NULL);
    delete generator;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    GeneratorPACSTest generator;

    // Default value for username
    BOOST_REQUIRE_EQUAL(generator.get_username(), "");
    // Set username
    generator.set_username("my_user");
    BOOST_REQUIRE_EQUAL(generator.get_username(), "my_user");
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Initialize function
 */
BOOST_FIXTURE_TEST_CASE(Initialize, ServicesTestClass)
{
    GeneratorPACSTest generator;
    BOOST_REQUIRE_EQUAL(generator.get_username(), "");

    dcmtkpp::Association association;
    association.set_user_identity_primary_field("my_user");
    dcmtkpp::message::CEchoRequest request(1, dcmtkpp::registry::VerificationSOPClass);

    auto status = generator.initialize(association, request);
    BOOST_REQUIRE(status == dcmtkpp::message::Response::Success);
    BOOST_REQUIRE_EQUAL(generator.get_username(), "my_user");

    BOOST_REQUIRE(!generator.is_connected());
    status = generator.initialize(mongo::BSONObj());
    BOOST_REQUIRE(status == dcmtkpp::message::Response::Success);
    BOOST_REQUIRE(generator.is_connected());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Done function
 */
BOOST_FIXTURE_TEST_CASE(Done, ServicesTestClass)
{
    GeneratorPACSTest generator;
    BOOST_REQUIRE(generator.done());

    generator.create_cursor();
    BOOST_REQUIRE(!generator.done());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Compute attribute function
 */
BOOST_FIXTURE_TEST_CASE(ComputeAttribute, ServicesTestClass)
{
    GeneratorPACSTest generator;

    auto status = generator.initialize(mongo::BSONObj());
    BOOST_REQUIRE(status == dcmtkpp::message::Response::Success);

    auto element = generator.compute_attribute(dcmtkpp::registry::InstanceAvailability, dcmtkpp::VR::CS, "");
    BOOST_REQUIRE(element == dcmtkpp::Element({"ONLINE"}, dcmtkpp::VR::CS));

    element = generator.compute_attribute(dcmtkpp::registry::ModalitiesInStudy, dcmtkpp::VR::CS, STUDY_INSTANCE_UID_01_01);
    BOOST_REQUIRE(element == dcmtkpp::Element({"MR"}, dcmtkpp::VR::CS));

    element = generator.compute_attribute(dcmtkpp::registry::NumberOfPatientRelatedStudies, dcmtkpp::VR::IS, "my_patient_id");
    BOOST_REQUIRE(element == dcmtkpp::Element({0}, dcmtkpp::VR::IS));

    element = generator.compute_attribute(dcmtkpp::registry::NumberOfPatientRelatedSeries, dcmtkpp::VR::IS, "my_patient_id");
    BOOST_REQUIRE(element == dcmtkpp::Element({0}, dcmtkpp::VR::IS));

    element = generator.compute_attribute(dcmtkpp::registry::NumberOfPatientRelatedInstances, dcmtkpp::VR::IS, "my_patient_id");
    BOOST_REQUIRE(element == dcmtkpp::Element({0}, dcmtkpp::VR::IS));

    element = generator.compute_attribute(dcmtkpp::registry::NumberOfStudyRelatedSeries, dcmtkpp::VR::IS, "my_patient_id");
    BOOST_REQUIRE(element == dcmtkpp::Element({0}, dcmtkpp::VR::IS));

    element = generator.compute_attribute(dcmtkpp::registry::NumberOfStudyRelatedInstances, dcmtkpp::VR::IS, "my_patient_id");
    BOOST_REQUIRE(element == dcmtkpp::Element({0}, dcmtkpp::VR::IS));

    element = generator.compute_attribute(dcmtkpp::registry::NumberOfSeriesRelatedInstances, dcmtkpp::VR::IS, "my_patient_id");
    BOOST_REQUIRE(element == dcmtkpp::Element({0}, dcmtkpp::VR::IS));

    element = generator.compute_attribute(dcmtkpp::registry::PatientName, dcmtkpp::VR::PN, "my_patient_name");
    BOOST_REQUIRE(element == dcmtkpp::Element());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Get Match type function
 */
BOOST_FIXTURE_TEST_CASE(GetMatchType, GeneratorPACSTest)
{
    // Single value
    test_get_match_type<std::string>("AE", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("AS", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("CS", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("DA", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<double>("DS", {12.34}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("DT", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<double>("FD", {12.34}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<double>("FL", {12.34}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<int>("IS", {1}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("LO", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("LT", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("SH", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("PN", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<int>("SL", {1}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<int>("SS", {1}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("ST", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("TM", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("UC", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<long long>("UL", {1}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<int>("US", {1}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    test_get_match_type<std::string>("UT", {"value"}, dopamine::services::GeneratorPACS::Match::Type::SingleValue);

    // VR == PN
    auto bson = BSON("key" << BSON("Alphabetic" << "value"));
    BOOST_REQUIRE(_get_match_type("PN", bson.getField("key")) == dopamine::services::GeneratorPACS::Match::Type::SingleValue);

    // Empty element
    mongo::BSONObjBuilder builder;
    builder.appendNull("key");
    BOOST_REQUIRE(_get_match_type("CS", builder.obj().getField("key")) == dopamine::services::GeneratorPACS::Match::Type::Universal);

    // Multiple Values
    bson = BSON("key" << BSON_ARRAY("value1" << "value2"));
    BOOST_REQUIRE(_get_match_type("CS", bson.getField("key")) == dopamine::services::GeneratorPACS::Match::Type::MultipleValues);

    // List of UID
    bson = BSON("key" << BSON_ARRAY("1.2.3" << "4.5.6"));
    BOOST_REQUIRE(_get_match_type("UI", bson.getField("key")) == dopamine::services::GeneratorPACS::Match::Type::ListOfUID);

    // Wild card
    test_get_match_type<std::string>("CS", {"v?l*e"}, dopamine::services::GeneratorPACS::Match::Type::WildCard);

    // Range
    test_get_match_type<std::string>("DA", {"value1-value2"}, dopamine::services::GeneratorPACS::Match::Type::Range);

    // Sequence
    bson = BSON("key" << BSON_ARRAY("value1" << "value2"));
    BOOST_REQUIRE(_get_match_type("SQ", bson.getField("key")) == dopamine::services::GeneratorPACS::Match::Type::Sequence);

    // No element
    BOOST_REQUIRE(_get_match_type("CS", mongo::BSONElement()) == dopamine::services::GeneratorPACS::Match::Type::Unknown);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: dicom_query_to_mongo_query function
 */
BOOST_FIXTURE_TEST_CASE(DicomQuery_to_MongoQuery, GeneratorPACSTest)
{
    // ListOfUID
    {
    DicomQueryToMongoQuery function_ = this->_get_query_conversion(dopamine::services::GeneratorPACS::Match::Type::ListOfUID);
    BOOST_REQUIRE(function_ != NULL);

        mongo::BSONObjBuilder builder_listofuid;
        (this->*function_)("00080018", "UI", BSON("key" << BSON_ARRAY("1.2.3" << "4.5.6")).getField("key"), builder_listofuid);
        mongo::BSONObj object_listofuid = builder_listofuid.obj();
        mongo::BSONObj const expected_result = BSON("$or" << BSON_ARRAY(BSON("00080018" << "1.2.3") << BSON("00080018" << "4.5.6")));
        BOOST_CHECK(object_listofuid.equal(expected_result));
    }

    // MultipleValues
    {
    DicomQueryToMongoQuery function_ = this->_get_query_conversion(dopamine::services::GeneratorPACS::Match::Type::MultipleValues);
    BOOST_REQUIRE(function_ != NULL);

    {
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "AE", BSON("key" << BSON_ARRAY("value" << "value2")).getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("$or" << BSON_ARRAY(BSON("00080060" << "value") << BSON("00080060" << "value2")));
        BOOST_CHECK(object_.equal(expected_result));
    }
    }

    // Range
    {
    DicomQueryToMongoQuery function_ = this->_get_query_conversion(dopamine::services::GeneratorPACS::Match::Type::Range);
    BOOST_REQUIRE(function_ != NULL);

    {
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "DA", BSON("key" << "value-value2").getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << BSON("$gte" << "value" << "$lte" << "value2"));
        BOOST_CHECK(object_.equal(expected_result));
    }
    }

    // Sequence
    {
    DicomQueryToMongoQuery function_ = this->_get_query_conversion(dopamine::services::GeneratorPACS::Match::Type::Sequence);
    BOOST_REQUIRE(function_ != NULL);
    // Not implemented
    }

    // SingleValue
    {
    DicomQueryToMongoQuery function_ = this->_get_query_conversion(dopamine::services::GeneratorPACS::Match::Type::SingleValue);
    BOOST_REQUIRE(function_ != NULL);

    {
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "AE", BSON("key" << "value").getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << "value");
        BOOST_CHECK(object_.equal(expected_result));
    }

    { // Special case for VR == PN
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "PN", BSON("key" << BSON("Alphabetic" << "value")).getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << "value");
        BOOST_CHECK(object_.equal(expected_result));
    }

    { // Special case for VR == DS
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "DS", BSON("key" << 12.34).getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << 12.34);
        BOOST_CHECK(object_.equal(expected_result));
    }

    { // Special case for VR == FD
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "FD", BSON("key" << 12.34).getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << 12.34);
        BOOST_CHECK(object_.equal(expected_result));
    }

    { // Special case for VR == FL
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "FL", BSON("key" << "12.34").getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << 12.34);
        //BOOST_CHECK(object_.equal(expected_result));
        BOOST_CHECK_CLOSE(object_.getField("00080060").numberDouble(), 12.34, 0.00001);
    }

    { // Special case for VR == IS
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "IS", BSON("key" << 12).getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << 12);
        BOOST_CHECK(object_.equal(expected_result));
    }

    { // Special case for VR == SL
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "SL", BSON("key" << 12).getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << 12);
        BOOST_CHECK(object_.equal(expected_result));
    }

    { // Special case for VR == SS
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "SS", BSON("key" << 12).getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << 12);
        BOOST_CHECK(object_.equal(expected_result));
    }

    { // Special case for VR == UL
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "UL", BSON("key" << 12).getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << 12);
        BOOST_CHECK(object_.equal(expected_result));
    }

    { // Special case for VR == US
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "US", BSON("key" << 12).getField("key"), builder);
        mongo::BSONObj object_ = builder.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << 12);
        BOOST_CHECK(object_.equal(expected_result));
    }
    }

    // Universal
    {
    DicomQueryToMongoQuery function_ = this->_get_query_conversion(dopamine::services::GeneratorPACS::Match::Type::Universal);
    BOOST_REQUIRE(function_ != NULL);

        mongo::BSONObjBuilder builder_universal;
        (this->*function_)("00080018", "UI", BSON("key" << "value").getField("key"), builder_universal);
        BOOST_REQUIRE(builder_universal.obj().isEmpty());
    }

    // Unknown
    {
    DicomQueryToMongoQuery function_ = this->_get_query_conversion(dopamine::services::GeneratorPACS::Match::Type::Unknown);
    BOOST_REQUIRE(function_ != NULL);

    { // Binary VR
        std::vector<uint8_t> value = { 0x1, 0x2, 0x3, 0x4, 0x5 };
        mongo::BSONObjBuilder binary_data_builder;
        binary_data_builder.appendBinData("00080060", value.size(),
                                          mongo::BinDataGeneral,
                                          (void*)(&value[0]));
        mongo::BSONObj const obj = binary_data_builder.obj();
        mongo::BSONObjBuilder builder;
        (this->*function_)("00080060", "OW", obj.getField("00080060"), builder);
        mongo::BSONObj object_ = builder.obj();
        //mongo::BSONObj const expected_result = BSON("00080060" << "");
        BOOST_CHECK(object_.equal(obj));
    }

    { // default
        mongo::BSONObjBuilder builder_unknown;
        (this->*function_)("00080060", "CS", BSON("key" << "value").getField("key"), builder_unknown);
        mongo::BSONObj object_unkown = builder_unknown.obj();
        mongo::BSONObj const expected_result = BSON("00080060" << "value");
        BOOST_CHECK(object_unkown.equal(expected_result));
    }
    }

    // WildCard
    {
    DicomQueryToMongoQuery function_ = this->_get_query_conversion(dopamine::services::GeneratorPACS::Match::Type::WildCard);
    BOOST_REQUIRE(function_ != NULL);

    {
        mongo::BSONObjBuilder builder_wilcard;
        (this->*function_)("00080018", "CS", BSON("key" << "v?lu*").getField("key"), builder_wilcard);
        mongo::BSONObj object_wilcard = builder_wilcard.obj();
        mongo::BSONObjBuilder expected_wilcard_builder;
        expected_wilcard_builder.appendRegex("00080018", "^v.lu.*$");
        mongo::BSONObj const expected_result = expected_wilcard_builder.obj();
        BOOST_CHECK(object_wilcard.equal(expected_result));
    }

    { // Special case for VR == PN
        mongo::BSONObjBuilder builder_wilcard;
        (this->*function_)("00080018", "PN", BSON("key" << BSON("Alphabetic" << "v?lu*")).getField("key"), builder_wilcard);
        mongo::BSONObj object_wilcard = builder_wilcard.obj();
        mongo::BSONObjBuilder expected_wilcard_builder;
        expected_wilcard_builder.appendRegex("00080018", "^v.lu.*$", "i");
        mongo::BSONObj const expected_result = expected_wilcard_builder.obj();
        BOOST_CHECK(object_wilcard.equal(expected_result));
    }
    }
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Bad connection
 *                  Status: ProcessingFailure
 */
BOOST_AUTO_TEST_CASE(ProcessingFailure)
{
    GeneratorPACSTest generator;
    auto status = generator.initialize(mongo::BSONObj());
    BOOST_REQUIRE_EQUAL(status,
                        dcmtkpp::message::Response::ProcessingFailure);
}
