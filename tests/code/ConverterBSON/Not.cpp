/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleNot
#include <boost/test/unit_test.hpp>

#include "ConverterBSON/AlwaysFalse.h"
#include "ConverterBSON/AlwaysTrue.h"
#include "ConverterBSON/Not.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Not True => False
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    auto alwaystrue = AlwaysTrue::New(); // we suppose AlwaysTrue correctly run
    auto not_ = Not::New(alwaystrue);
    
    BOOST_CHECK_EQUAL((*not_)(NULL), false);
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Not False => True
 */
BOOST_AUTO_TEST_CASE(TEST_OK_02)
{
    auto alwaysfalse = AlwaysFalse::New(); // we suppose AlwaysFalse correctly run
    auto not_ = Not::New(alwaysfalse);
    
    BOOST_CHECK_EQUAL((*not_)(NULL), true);
}
