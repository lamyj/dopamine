/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleTagMatch
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/Tag.h>

#include "ConverterBSON/TagMatch.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Tag Match
 */
BOOST_AUTO_TEST_CASE(Matching)
{
    auto tagmatch = dopamine::converterBSON::
            TagMatch::New(dcmtkpp::Tag(0x0010, 0x0010));

    BOOST_CHECK_EQUAL((*tagmatch)(dcmtkpp::Tag(0x0010, 0x0010),
                                  dcmtkpp::Element()), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Tag not Match => different Group
 */
BOOST_AUTO_TEST_CASE(DifferentGroup)
{
    auto tagmatch = dopamine::converterBSON::
            TagMatch::New(dcmtkpp::Tag(0x0008, 0x0010));

    BOOST_CHECK_EQUAL((*tagmatch)(dcmtkpp::Tag(0x0010, 0x0010),
                                  dcmtkpp::Element()), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Tag not Match => different Element
 */
BOOST_AUTO_TEST_CASE(DifferentElement)
{
    auto tagmatch = dopamine::converterBSON::
            TagMatch::New(dcmtkpp::Tag(0x0010, 0x0020));

    BOOST_CHECK_EQUAL((*tagmatch)(dcmtkpp::Tag(0x0010, 0x0010),
                                  dcmtkpp::Element()), false);
}
