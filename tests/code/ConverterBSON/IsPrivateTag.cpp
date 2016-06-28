/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE IsPrivateTag
#include <boost/test/unit_test.hpp>

#include "ConverterBSON/IsPrivateTag.h"

BOOST_AUTO_TEST_CASE(PrivateTag)
{
    dopamine::converterBSON::IsPrivateTag isprivatetag;

    BOOST_CHECK(isprivatetag(odil::Tag("00231001"), odil::Element()));
}

BOOST_AUTO_TEST_CASE(PublicTag)
{
    dopamine::converterBSON::IsPrivateTag isprivatetag;

    BOOST_CHECK(!isprivatetag(odil::Tag("00100010"), odil::Element()));
}
