/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE AlwaysFalse
#include <boost/test/unit_test.hpp>

#include "ConverterBSON/AlwaysFalse.h"

BOOST_AUTO_TEST_CASE(Evaluate)
{
    dopamine::converterBSON::AlwaysFalse alwaysfalse;
    
    BOOST_CHECK(!alwaysfalse(odil::Tag("deadbeef"), odil::Element()));
}
