/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleVRMatch
#include <boost/test/unit_test.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "ConverterBSON/VRMatch.h"

/*************************** TEST OK 01 *******************************/
/**
 * Nominal test case: VR Match
 */
BOOST_AUTO_TEST_CASE(TEST_OK_01)
{
    auto vrmatch = VRMatch::New(DcmVR("PN").getEVR());
    
    DcmElement* element = new DcmAttributeTag(DcmTag(0x0010, 0x0010));
    BOOST_CHECK_EQUAL((*vrmatch)(element), true);
    
    delete element;
}

/*************************** TEST OK 02 *******************************/
/**
 * Nominal test case: VR not Match
 */
BOOST_AUTO_TEST_CASE(TEST_OK_02)
{
    auto vrmatch = VRMatch::New(DcmVR("PN").getEVR());
    
    DcmElement* element = new DcmAttributeTag(DcmTag(0x0010, 0x0020));
    BOOST_CHECK_EQUAL((*vrmatch)(element), false);
    
    delete element;
}

/*************************** TEST OK 03 *******************************/
/**
 * Nominal test case: VR not Match => tested element is empty
 */
BOOST_AUTO_TEST_CASE(TEST_OK_03)
{
    auto vrmatch = VRMatch::New(DcmVR("PN").getEVR());
    
    BOOST_CHECK_EQUAL((*vrmatch)(NULL), false);
}
