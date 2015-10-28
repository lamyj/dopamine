/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleNot
#include <boost/test/unit_test.hpp>

#include "ConditionDataTest.h"
#include "ConverterBSON/Not.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Not True => False
 */
BOOST_FIXTURE_TEST_CASE(NotTrue, ConditionDataTest)
{
    auto not_ = dopamine::converterBSON::Not::New(alwaystrue);
    
    BOOST_CHECK_EQUAL((*not_)(dcmtkpp::Tag("deadbeef"),
                              dcmtkpp::Element()), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Not False => True
 */
BOOST_FIXTURE_TEST_CASE(NotFalse, ConditionDataTest)
{
    auto not_ = dopamine::converterBSON::Not::New(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*not_)(dcmtkpp::Tag("deadbeef"),
                              dcmtkpp::Element()), true);
}
