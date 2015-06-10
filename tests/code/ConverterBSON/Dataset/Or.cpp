/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleOr
#include <boost/test/unit_test.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>
#include <dcmtk/dcmdata/dcvrat.h>

#include "ConverterBSON/Dataset/AlwaysFalse.h"
#include "ConverterBSON/Dataset/AlwaysTrue.h"
#include "ConverterBSON/Dataset/Or.h"
#include "core/ExceptionPACS.h"

struct TestDataOrOperator
{
    DcmElement * element;
    dopamine::converterBSON::AlwaysTrue::Pointer alwaystrue;
    dopamine::converterBSON::AlwaysFalse::Pointer alwaysfalse;
 
    TestDataOrOperator()
    {
        element     = new DcmAttributeTag(DcmTag(0010,0010));
        alwaystrue  = dopamine::converterBSON::AlwaysTrue::New();  // we suppose AlwaysTrue correctly run
        alwaysfalse = dopamine::converterBSON::AlwaysFalse::New(); // we suppose AlwaysFalse correctly run
    }
 
    ~TestDataOrOperator()
    {
        delete element;
    }
};

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: True Or True => True
 */
BOOST_FIXTURE_TEST_CASE(TrueOrTrue, TestDataOrOperator)
{
    auto or_ = dopamine::converterBSON::Or::New();
    or_->insert_condition(alwaystrue);
    or_->insert_condition(alwaystrue);
    
    BOOST_CHECK_EQUAL((*or_)(element), true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: True Or False => True
 */
BOOST_FIXTURE_TEST_CASE(TrueOrFalse, TestDataOrOperator)
{
    auto or_ = dopamine::converterBSON::Or::New();
    or_->insert_condition(alwaystrue);
    or_->insert_condition(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*or_)(element), true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: False Or True => True
 */
BOOST_FIXTURE_TEST_CASE(FalseOrTrue, TestDataOrOperator)
{
    auto or_ = dopamine::converterBSON::Or::New();
    or_->insert_condition(alwaysfalse);
    or_->insert_condition(alwaystrue);
    
    BOOST_CHECK_EQUAL((*or_)(element), true);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: False Or False => False
 */
BOOST_FIXTURE_TEST_CASE(FalseOrFalse, TestDataOrOperator)
{
    auto or_ = dopamine::converterBSON::Or::New();
    or_->insert_condition(alwaysfalse);
    or_->insert_condition(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*or_)(element), false);
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Element is null
 */
BOOST_FIXTURE_TEST_CASE(EmptyElement, TestDataOrOperator)
{
    auto or_ = dopamine::converterBSON::Or::New();
    or_->insert_condition(alwaystrue);
    or_->insert_condition(alwaystrue);
    
    BOOST_REQUIRE_THROW((*or_)(NULL), dopamine::ExceptionPACS);
}
