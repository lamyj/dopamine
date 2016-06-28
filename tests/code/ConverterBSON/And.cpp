/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE And
#include <boost/test/unit_test.hpp>

#include "ConditionDataTest.h"
#include "ConverterBSON/And.h"

BOOST_FIXTURE_TEST_CASE(TrueAndTrue, ConditionDataTest)
{
    dopamine::converterBSON::And and_;
    and_.get_terms().push_back(alwaystrue);
    and_.get_terms().push_back(alwaystrue);
    
    BOOST_CHECK(and_(odil::Tag("deadbeef"), odil::Element()));
}

BOOST_FIXTURE_TEST_CASE(TrueAndFalse, ConditionDataTest)
{
    dopamine::converterBSON::And and_;
    and_.get_terms().push_back(alwaystrue);
    and_.get_terms().push_back(alwaysfalse);

    BOOST_CHECK(!and_(odil::Tag("deadbeef"), odil::Element()));
}

BOOST_FIXTURE_TEST_CASE(FalseAndTrue, ConditionDataTest)
{
    dopamine::converterBSON::And and_;
    and_.get_terms().push_back(alwaysfalse);
    and_.get_terms().push_back(alwaystrue);

    BOOST_CHECK(!and_(odil::Tag("deadbeef"), odil::Element()));
}

BOOST_FIXTURE_TEST_CASE(FalseAndFalse, ConditionDataTest)
{
    dopamine::converterBSON::And and_;
    and_.get_terms().push_back(alwaysfalse);
    and_.get_terms().push_back(alwaysfalse);

    BOOST_CHECK(!and_(odil::Tag("deadbeef"), odil::Element()));
}
