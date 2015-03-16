/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleEchoSCP
#include <boost/test/unit_test.hpp>

#include "SCP/EchoSCP.h"

/**
 * Pre-conditions:
 *
 */

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor/Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    dopamine::EchoSCP * echoscp =
            new dopamine::EchoSCP(NULL, T_ASC_PresentationContextID(), NULL);

    BOOST_REQUIRE_EQUAL(echoscp != NULL, true);

    delete echoscp;
}
