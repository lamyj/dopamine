/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "dopamine/utils.h"

#include <string>

namespace dopamine
{

std::string
replace(
    std::string const & value, std::string const & old, std::string const & new_)
{
    std::string result(value);
    size_t begin=0;
    while(std::string::npos != (begin=result.find(old, begin)))
    {
        result = result.replace(begin, old.size(), new_);
        if(begin+new_.size()<result.size())
        {
            begin = begin+new_.size();
        }
        else
        {
            begin = std::string::npos;
        }
    }

    return result;
}

} // namespace dopamine
