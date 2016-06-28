/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE TagMatch
#include <boost/test/unit_test.hpp>

#include <odil/Tag.h>

#include "ConverterBSON/TagMatch.h"

BOOST_AUTO_TEST_CASE(Matching)
{
    dopamine::converterBSON::TagMatch tagmatch(odil::Tag(0x0010, 0x0010));

    BOOST_CHECK(tagmatch(odil::Tag(0x0010, 0x0010), odil::Element()));
}

BOOST_AUTO_TEST_CASE(DifferentGroup)
{
    dopamine::converterBSON::TagMatch tagmatch(odil::Tag(0x0008, 0x0010));

    BOOST_CHECK(!tagmatch(odil::Tag(0x0010, 0x0010), odil::Element()));
}

BOOST_AUTO_TEST_CASE(DifferentElement)
{
    dopamine::converterBSON::TagMatch tagmatch(odil::Tag(0x0010, 0x0020));

    BOOST_CHECK(!tagmatch(odil::Tag(0x0010, 0x0010), odil::Element()));
}
