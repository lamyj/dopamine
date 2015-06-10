/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleVRMatch
#include <boost/test/unit_test.hpp>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "ConverterBSON/Dataset/VRMatch.h"

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: VR Match
 */
BOOST_AUTO_TEST_CASE(Matching)
{
    auto vrmatch = dopamine::converterBSON::VRMatch::New(DcmVR("PN").getEVR());
    
    DcmElement* element = new DcmAttributeTag(DcmTag(0x0010, 0x0010));
    BOOST_CHECK_EQUAL((*vrmatch)(element), true);
    
    delete element;
}

/*************************** TEST Nominal *******************************/
/**
 * Nominal test case: VR not Match
 */
BOOST_AUTO_TEST_CASE(NotMatching)
{
    auto vrmatch = dopamine::converterBSON::VRMatch::New(DcmVR("PN").getEVR());
    
    DcmElement* element = new DcmAttributeTag(DcmTag(0x0010, 0x0020));
    BOOST_CHECK_EQUAL((*vrmatch)(element), false);
    
    delete element;
}

/*************************** TEST Error *********************************/
/**
 * Error test case: Element is null
 */
BOOST_AUTO_TEST_CASE(EmptyElement)
{
    auto vrmatch = dopamine::converterBSON::VRMatch::New(DcmVR("PN").getEVR());
    
    BOOST_REQUIRE_THROW((*vrmatch)(NULL), dopamine::ExceptionPACS);
}
