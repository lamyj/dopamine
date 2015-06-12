/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleNot
#include <boost/test/unit_test.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcelem.h>
#include <dcmtk/dcmdata/dcvrat.h>

#include "ConverterBSON/Dataset/AlwaysFalse.h"
#include "ConverterBSON/Dataset/AlwaysTrue.h"
#include "ConverterBSON/Dataset/Not.h"
#include "core/ExceptionPACS.h"

struct TestDataNotOperator
{
    DcmElement * element;
    dopamine::converterBSON::AlwaysTrue::Pointer  alwaystrue;
    dopamine::converterBSON::AlwaysFalse::Pointer alwaysfalse;
 
    TestDataNotOperator()
    {
        element     = new DcmAttributeTag(DcmTag(0010,0010));
        // we suppose AlwaysTrue correctly run
        alwaystrue  = dopamine::converterBSON::AlwaysTrue::New();
        // we suppose AlwaysFalse correctly run
        alwaysfalse = dopamine::converterBSON::AlwaysFalse::New();
    }
 
    ~TestDataNotOperator()
    {
        delete element;
    }
};

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Not True => False
 */
BOOST_FIXTURE_TEST_CASE(NotTrue, TestDataNotOperator)
{
    auto not_ = dopamine::converterBSON::Not::New(alwaystrue);
    
    BOOST_CHECK_EQUAL((*not_)(element), false);
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: Not False => True
 */
BOOST_FIXTURE_TEST_CASE(NotFalse, TestDataNotOperator)
{
    auto not_ = dopamine::converterBSON::Not::New(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*not_)(element), true);
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Element is null
 */
BOOST_FIXTURE_TEST_CASE(EmptyElement, TestDataNotOperator)
{
    auto not_ = dopamine::converterBSON::Not::New(alwaystrue);
    
    BOOST_REQUIRE_THROW((*not_)(NULL), dopamine::ExceptionPACS);
}
