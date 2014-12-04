/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleAlwaysTrue
#include <boost/test/unit_test.hpp>

#include "ConverterBSON/AlwaysTrue.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    auto alwaystrue = research_pacs::AlwaysTrue::New();
    
    BOOST_CHECK_EQUAL((*alwaystrue)(NULL), true);
}
