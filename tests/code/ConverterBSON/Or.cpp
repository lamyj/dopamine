/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleOr
#include <boost/test/unit_test.hpp>

#include "ConverterBSON/AlwaysFalse.h"
#include "ConverterBSON/AlwaysTrue.h"
#include "ConverterBSON/Or.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: True Or True => True
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    auto alwaystrue = AlwaysTrue::New(); // we suppose AlwaysTrue correctly run
    auto or_ = Or::New();
    or_->conditions.push_back(alwaystrue);
    or_->conditions.push_back(alwaystrue);
    
    BOOST_CHECK_EQUAL((*or_)(NULL), true);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: True Or False => True
 */
BOOST_AUTO_TEST_CASE(TEST_OK_02)
{
    auto alwaystrue = AlwaysTrue::New(); // we suppose AlwaysTrue correctly run
    auto alwaysfalse = AlwaysFalse::New(); // we suppose AlwaysFalse correctly run
    auto or_ = Or::New();
    or_->conditions.push_back(alwaystrue);
    or_->conditions.push_back(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*or_)(NULL), true);
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: False Or True => True
 */
BOOST_AUTO_TEST_CASE(TEST_OK_03)
{
    auto alwaystrue = AlwaysTrue::New(); // we suppose AlwaysTrue correctly run
    auto alwaysfalse = AlwaysFalse::New(); // we suppose AlwaysFalse correctly run
    auto or_ = Or::New();
    or_->conditions.push_back(alwaysfalse);
    or_->conditions.push_back(alwaystrue);
    
    BOOST_CHECK_EQUAL((*or_)(NULL), true);
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: False Or False => False
 */
BOOST_AUTO_TEST_CASE(TEST_OK_04)
{
    auto alwaysfalse = AlwaysFalse::New(); // we suppose AlwaysFalse correctly run
    auto or_ = Or::New();
    or_->conditions.push_back(alwaysfalse);
    or_->conditions.push_back(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*or_)(NULL), false);
}

