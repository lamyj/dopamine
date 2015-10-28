/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _a09d7b94_c9dd_4fcf_ab11_03b8622f37b3
#define _a09d7b94_c9dd_4fcf_ab11_03b8622f37b3

#include "ConverterBSON/AlwaysFalse.h"
#include "ConverterBSON/AlwaysTrue.h"

struct ConditionDataTest
{
    dopamine::converterBSON::AlwaysTrue::Pointer  alwaystrue;
    dopamine::converterBSON::AlwaysFalse::Pointer alwaysfalse;

    ConditionDataTest()
    {
        // we suppose AlwaysTrue correctly run
        alwaystrue  = dopamine::converterBSON::AlwaysTrue::New();
        // we suppose AlwaysFalse correctly run
        alwaysfalse = dopamine::converterBSON::AlwaysFalse::New();
    }

    ~ConditionDataTest()
    {
        // Nothing to do
    }
};

#endif // _a09d7b94_c9dd_4fcf_ab11_03b8622f37b3
