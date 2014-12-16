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
#include <dcmtk/dcmdata/dctk.h>

#include "ConverterBSON/AlwaysFalse.h"
#include "ConverterBSON/AlwaysTrue.h"
#include "ConverterBSON/Or.h"
#include "core/ExceptionPACS.h"

struct TestDataOK01
{
    DcmElement * element;
    dopamine::AlwaysTrue::Pointer alwaystrue;
    dopamine::AlwaysFalse::Pointer alwaysfalse;
 
    TestDataOK01()
    {
        element     = new DcmAttributeTag(DcmTag(0010,0010));
        alwaystrue  = dopamine::AlwaysTrue::New();  // we suppose AlwaysTrue correctly run
        alwaysfalse = dopamine::AlwaysFalse::New(); // we suppose AlwaysFalse correctly run
    }
 
    ~TestDataOK01()
    {
        delete element;
    }
};

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: True Or True => True
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_01, TestDataOK01)
{
    auto or_ = dopamine::Or::New();
    or_->insert_condition(alwaystrue);
    or_->insert_condition(alwaystrue);
    
    BOOST_CHECK_EQUAL((*or_)(element), true);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: True Or False => True
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK01)
{
    auto or_ = dopamine::Or::New();
    or_->insert_condition(alwaystrue);
    or_->insert_condition(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*or_)(element), true);
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: False Or True => True
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_03, TestDataOK01)
{
    auto or_ = dopamine::Or::New();
    or_->insert_condition(alwaysfalse);
    or_->insert_condition(alwaystrue);
    
    BOOST_CHECK_EQUAL((*or_)(element), true);
}

/*************************** TEST OK 04 *******************************/
/**
 * Nominal test case: False Or False => False
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_04, TestDataOK01)
{
    auto or_ = dopamine::Or::New();
    or_->insert_condition(alwaysfalse);
    or_->insert_condition(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*or_)(element), false);
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Element is null
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_01, TestDataOK01)
{
    auto or_ = dopamine::Or::New();
    or_->insert_condition(alwaystrue);
    or_->insert_condition(alwaystrue);
    
    BOOST_REQUIRE_THROW((*or_)(NULL), dopamine::ExceptionPACS);
}
