/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleIsPrivateTag
#include <boost/test/unit_test.hpp>

#include "ConverterBSON/IsPrivateTag.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Private Tag
 */
BOOST_AUTO_TEST_CASE(PrivateTag)
{
    auto isprivatetag = dopamine::converterBSON::IsPrivateTag::New();

    BOOST_CHECK_EQUAL((*isprivatetag)(dcmtkpp::Tag("00231001"),
                                      dcmtkpp::Element()), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Public Tag
 */
BOOST_AUTO_TEST_CASE(PublicTag)
{
    auto isprivatetag = dopamine::converterBSON::IsPrivateTag::New();

    BOOST_CHECK_EQUAL((*isprivatetag)(dcmtkpp::Tag("00100010"),
                                      dcmtkpp::Element()), false);
}
