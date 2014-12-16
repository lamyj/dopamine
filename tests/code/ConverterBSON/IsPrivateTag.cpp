/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleIsPrivateTag
#include <boost/test/unit_test.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "ConverterBSON/IsPrivateTag.h"
#include "core/ExceptionPACS.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: Private Tag
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    auto isprivatetag = dopamine::IsPrivateTag::New();
    
    DcmElement* element = new DcmAttributeTag(DcmTag(0023,1001));
    BOOST_CHECK_EQUAL((*isprivatetag)(element), true);
    
    delete element;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: Public Tag
 */
BOOST_AUTO_TEST_CASE(TEST_OK_02)
{
    auto isprivatetag = dopamine::IsPrivateTag::New();
    
    DcmElement* element = new DcmAttributeTag(DcmTag(0010,0010));
    BOOST_CHECK_EQUAL((*isprivatetag)(element), false);
    
    delete element;
}

/*************************** TEST KO 01 *******************************/
/**
 * Error test case: Empty element
 */
BOOST_AUTO_TEST_CASE(TEST_KO_01)
{
    auto isprivatetag = dopamine::IsPrivateTag::New();
    
    BOOST_REQUIRE_THROW((*isprivatetag)(NULL), dopamine::ExceptionPACS);
}
