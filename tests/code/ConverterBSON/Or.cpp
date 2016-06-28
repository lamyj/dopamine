/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE Or
#include <boost/test/unit_test.hpp>

#include "ConditionDataTest.h"
#include "ConverterBSON/Or.h"

BOOST_FIXTURE_TEST_CASE(TrueOrTrue, ConditionDataTest)
{
    dopamine::converterBSON::Or or_;
    or_.get_terms().push_back(alwaystrue);
    or_.get_terms().push_back(alwaystrue);

    BOOST_CHECK(or_(odil::Tag("deadbeef"), odil::Element()));
}

BOOST_FIXTURE_TEST_CASE(TrueOrFalse, ConditionDataTest)
{
    dopamine::converterBSON::Or or_;
    or_.get_terms().push_back(alwaystrue);
    or_.get_terms().push_back(alwaysfalse);

    BOOST_CHECK(or_(odil::Tag("deadbeef"), odil::Element()));
}

BOOST_FIXTURE_TEST_CASE(FalseOrTrue, ConditionDataTest)
{
    dopamine::converterBSON::Or or_;
    or_.get_terms().push_back(alwaysfalse);
    or_.get_terms().push_back(alwaystrue);

    BOOST_CHECK(or_(odil::Tag("deadbeef"), odil::Element()));
}

BOOST_FIXTURE_TEST_CASE(FalseOrFalse, ConditionDataTest)
{
    dopamine::converterBSON::Or or_;
    or_.get_terms().push_back(alwaysfalse);
    or_.get_terms().push_back(alwaysfalse);

    BOOST_CHECK(!or_(odil::Tag("deadbeef"), odil::Element()));
}
