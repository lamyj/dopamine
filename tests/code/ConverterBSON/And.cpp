/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleAnd
#include <boost/test/unit_test.hpp>

#include "ConditionDataTest.h"
#include "ConverterBSON/And.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: True And True => True
 */
BOOST_FIXTURE_TEST_CASE(TrueAndTrue, ConditionDataTest)
{
    auto and_ = dopamine::converterBSON::And::New();
    and_->insert_condition(alwaystrue);
    and_->insert_condition(alwaystrue);
    
    BOOST_CHECK_EQUAL((*and_)(dcmtkpp::Tag("deadbeef"),
                              dcmtkpp::Element()), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: True And False => False
 */
BOOST_FIXTURE_TEST_CASE(TrueAndFalse, ConditionDataTest)
{
    auto and_ = dopamine::converterBSON::And::New();
    and_->insert_condition(alwaystrue);
    and_->insert_condition(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*and_)(dcmtkpp::Tag("deadbeef"),
                              dcmtkpp::Element()), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: False And True => False
 */
BOOST_FIXTURE_TEST_CASE(FalseAndTrue, ConditionDataTest)
{
    auto and_ = dopamine::converterBSON::And::New();
    and_->insert_condition(alwaysfalse);
    and_->insert_condition(alwaystrue);
    
    BOOST_CHECK_EQUAL((*and_)(dcmtkpp::Tag("deadbeef"),
                              dcmtkpp::Element()), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: False And False => False
 */
BOOST_FIXTURE_TEST_CASE(FalseAndFalse, ConditionDataTest)
{
    auto and_ = dopamine::converterBSON::And::New();
    and_->insert_condition(alwaysfalse);
    and_->insert_condition(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*and_)(dcmtkpp::Tag("deadbeef"),
                              dcmtkpp::Element()), false);
}
