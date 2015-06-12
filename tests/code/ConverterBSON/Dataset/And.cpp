/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleAnd
#include <boost/test/unit_test.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>
#include <dcmtk/dcmdata/dcvrat.h>

#include "ConverterBSON/Dataset/AlwaysFalse.h"
#include "ConverterBSON/Dataset/AlwaysTrue.h"
#include "ConverterBSON/Dataset/And.h"
#include "core/ExceptionPACS.h"

struct TestDataAndOperator
{
    DcmElement * element;
    dopamine::converterBSON::AlwaysTrue::Pointer  alwaystrue;
    dopamine::converterBSON::AlwaysFalse::Pointer alwaysfalse;
 
    TestDataAndOperator()
    {
        element     = new DcmAttributeTag(DcmTag(0010,0010));
        // we suppose AlwaysTrue correctly run
        alwaystrue  = dopamine::converterBSON::AlwaysTrue::New();
        // we suppose AlwaysFalse correctly run
        alwaysfalse = dopamine::converterBSON::AlwaysFalse::New();
    }

    TestDataAndOperator(TestDataAndOperator const & other)
    {
        element     = new DcmAttributeTag(DcmTag(0010,0010));
        alwaystrue  = dopamine::converterBSON::AlwaysTrue::New();
        alwaysfalse = dopamine::converterBSON::AlwaysFalse::New();
    }
 
    ~TestDataAndOperator()
    {
        delete element;
    }
};

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: True And True => True
 */
BOOST_FIXTURE_TEST_CASE(TrueAndTrue, TestDataAndOperator)
{
    auto and_ = dopamine::converterBSON::And::New();
    and_->insert_condition(alwaystrue);
    and_->insert_condition(alwaystrue);
    
    BOOST_CHECK_EQUAL((*and_)(element), true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: True And False => False
 */
BOOST_FIXTURE_TEST_CASE(TrueAndFalse, TestDataAndOperator)
{
    auto and_ = dopamine::converterBSON::And::New();
    and_->insert_condition(alwaystrue);
    and_->insert_condition(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*and_)(element), false);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: False And True => False
 */
BOOST_FIXTURE_TEST_CASE(FalseAndTrue, TestDataAndOperator)
{
    auto and_ = dopamine::converterBSON::And::New();
    and_->insert_condition(alwaysfalse);
    and_->insert_condition(alwaystrue);
    
    BOOST_CHECK_EQUAL((*and_)(element), false);
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: False And False => False
 */
BOOST_FIXTURE_TEST_CASE(FalseAndFalse, TestDataAndOperator)
{
    auto and_ = dopamine::converterBSON::And::New();
    and_->insert_condition(alwaysfalse);
    and_->insert_condition(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*and_)(element), false);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Element is null
 */
BOOST_FIXTURE_TEST_CASE(EmptyElement, TestDataAndOperator)
{
    auto and_ = dopamine::converterBSON::And::New();
    and_->insert_condition(alwaystrue);
    and_->insert_condition(alwaystrue);
    
    BOOST_REQUIRE_THROW((*and_)(NULL), dopamine::ExceptionPACS);
}
