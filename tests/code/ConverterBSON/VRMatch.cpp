/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE VRMatch
#include <boost/test/unit_test.hpp>

#include <odil/Element.h>
#include <odil/Tag.h>
#include <odil/VR.h>

#include "ConverterBSON/VRMatch.h"

BOOST_AUTO_TEST_CASE(Matching)
{
    dopamine::converterBSON::VRMatch vrmatch(odil::VR::PN);

    BOOST_CHECK(
        vrmatch(
            odil::Tag(0x0010, 0x0010),
            odil::Element(odil::Value::Strings(),  odil::VR::PN)));
}

BOOST_AUTO_TEST_CASE(NotMatching)
{
    dopamine::converterBSON::VRMatch vrmatch(odil::VR::PN);

    BOOST_CHECK(
        !vrmatch(
            odil::Tag(0x0010, 0x0020),
            odil::Element(odil::Value::Strings(), odil::VR::LO)));
}
