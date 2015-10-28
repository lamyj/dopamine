/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleOr
#include <boost/test/unit_test.hpp>

#include "ConditionDataTest.h"
#include "ConverterBSON/Or.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: True Or True => True
 */
BOOST_FIXTURE_TEST_CASE(TrueOrTrue, ConditionDataTest)
{
    auto or_ = dopamine::converterBSON::Or::New();
    or_->insert_condition(alwaystrue);
    or_->insert_condition(alwaystrue);
    
    BOOST_CHECK_EQUAL((*or_)(dcmtkpp::Tag("deadbeef"),
                             dcmtkpp::Element()), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: True Or False => True
 */
BOOST_FIXTURE_TEST_CASE(TrueOrFalse, ConditionDataTest)
{
    auto or_ = dopamine::converterBSON::Or::New();
    or_->insert_condition(alwaystrue);
    or_->insert_condition(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*or_)(dcmtkpp::Tag("deadbeef"),
                             dcmtkpp::Element()), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: False Or True => True
 */
BOOST_FIXTURE_TEST_CASE(FalseOrTrue, ConditionDataTest)
{
    auto or_ = dopamine::converterBSON::Or::New();
    or_->insert_condition(alwaysfalse);
    or_->insert_condition(alwaystrue);
    
    BOOST_CHECK_EQUAL((*or_)(dcmtkpp::Tag("deadbeef"),
                             dcmtkpp::Element()), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: False Or False => False
 */
BOOST_FIXTURE_TEST_CASE(FalseOrFalse, ConditionDataTest)
{
    auto or_ = dopamine::converterBSON::Or::New();
    or_->insert_condition(alwaysfalse);
    or_->insert_condition(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*or_)(dcmtkpp::Tag("deadbeef"),
                             dcmtkpp::Element()), false);
}
