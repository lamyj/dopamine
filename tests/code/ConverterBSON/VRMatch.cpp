/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleVRMatch
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/Element.h>
#include <dcmtkpp/Tag.h>
#include <dcmtkpp/VR.h>

#include "ConverterBSON/VRMatch.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: VR Match
 */
BOOST_AUTO_TEST_CASE(Matching)
{
    auto vrmatch = dopamine::converterBSON::VRMatch::New(dcmtkpp::VR::PN);

    BOOST_CHECK_EQUAL((*vrmatch)(dcmtkpp::Tag(0x0010, 0x0010),
                                 dcmtkpp::Element(dcmtkpp::Value::Strings(),
                                                  dcmtkpp::VR::PN)), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: VR not Match
 */
BOOST_AUTO_TEST_CASE(NotMatching)
{
    auto vrmatch = dopamine::converterBSON::VRMatch::New(dcmtkpp::VR::PN);

    BOOST_CHECK_EQUAL((*vrmatch)(dcmtkpp::Tag(0x0010, 0x0020),
                                 dcmtkpp::Element(dcmtkpp::Value::Strings(),
                                                  dcmtkpp::VR::LO)), false);
}
