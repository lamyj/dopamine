/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _a09d7b94_c9dd_4fcf_ab11_03b8622f37b3
#define _a09d7b94_c9dd_4fcf_ab11_03b8622f37b3

#include <memory>

#include "ConverterBSON/AlwaysFalse.h"
#include "ConverterBSON/AlwaysTrue.h"

struct ConditionDataTest
{
    std::shared_ptr<dopamine::converterBSON::AlwaysTrue> alwaystrue;
    std::shared_ptr<dopamine::converterBSON::AlwaysFalse> alwaysfalse;

    ConditionDataTest()
    {
        // we suppose AlwaysTrue and AlwaysFalse correctly run
        alwaystrue = std::make_shared<dopamine::converterBSON::AlwaysTrue>();
        alwaysfalse = std::make_shared<dopamine::converterBSON::AlwaysFalse>();
    }
};

#endif // _a09d7b94_c9dd_4fcf_ab11_03b8622f37b3
