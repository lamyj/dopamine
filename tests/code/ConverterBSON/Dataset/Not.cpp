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
#include <dcmtk/dcmdata/dctk.h>

#include "ConverterBSON/Dataset/AlwaysFalse.h"
#include "ConverterBSON/Dataset/AlwaysTrue.h"
#include "ConverterBSON/Dataset/Not.h"
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
 * Nominal test case: Not True => False
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_01, TestDataOK01)
{
    auto not_ = dopamine::Not::New(alwaystrue);
    
    BOOST_CHECK_EQUAL((*not_)(element), false);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Not False => True
 */
BOOST_FIXTURE_TEST_CASE(TEST_OK_02, TestDataOK01)
{
    auto not_ = dopamine::Not::New(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*not_)(element), true);
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Element is null
 */
BOOST_FIXTURE_TEST_CASE(TEST_KO_01, TestDataOK01)
{
    auto not_ = dopamine::Not::New(alwaystrue);
    
    BOOST_REQUIRE_THROW((*not_)(NULL), dopamine::ExceptionPACS);
}
