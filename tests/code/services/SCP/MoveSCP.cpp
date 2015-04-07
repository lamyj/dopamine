/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleMoveSCP
#include <boost/test/unit_test.hpp>

#include "services/SCP/MoveSCP.h"

/**
 * Pre-conditions:
 *
 */

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Constructor
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    dopamine::services::MoveSCP * movescp =
            new dopamine::services::MoveSCP(NULL,
                                            T_ASC_PresentationContextID(),
                                            NULL);

    BOOST_REQUIRE_EQUAL(movescp != NULL, true);

    delete movescp;
}
