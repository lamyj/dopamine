/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleExceptionPACS
#include <boost/test/unit_test.hpp>

#include "core/ExceptionPACS.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Throw exception
 */
BOOST_AUTO_TEST_CASE(ThrowException)
{
    bool isthrow = false;

    try
    {
        throw dopamine::ExceptionPACS("MyText");
    }
    catch (dopamine::ExceptionPACS& exception)
    {
        BOOST_CHECK_EQUAL(exception.what(), "MyText");
        isthrow = true;
    }

    BOOST_REQUIRE(isthrow);
}
