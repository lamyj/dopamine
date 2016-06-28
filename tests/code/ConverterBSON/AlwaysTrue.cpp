/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE AlwaysTrue
#include <boost/test/unit_test.hpp>

#include "ConverterBSON/AlwaysTrue.h"

BOOST_AUTO_TEST_CASE(Evaluate)
{
    dopamine::converterBSON::AlwaysTrue alwaystrue;

    BOOST_CHECK(alwaystrue(odil::Tag("deadbeef"), odil::Element()));
}
