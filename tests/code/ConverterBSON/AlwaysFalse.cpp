/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleAlwaysFalse
#include <boost/test/unit_test.hpp>

#include "ConverterBSON/AlwaysFalse.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case
 */
BOOST_AUTO_TEST_CASE(Evaluate)
{
    auto alwaysfalse = dopamine::converterBSON::AlwaysFalse::New();
    
    BOOST_CHECK_EQUAL((*alwaysfalse)(dcmtkpp::Tag("deadbeef"),
                                     dcmtkpp::Element()), false);
}
