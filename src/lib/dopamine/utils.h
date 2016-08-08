/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _da25cc04_b8bb_4e69_8cd2_b27f5acf27fc
#define _da25cc04_b8bb_4e69_8cd2_b27f5acf27fc

#include <string>

namespace dopamine
{

/// @brief Replace every occurence of old by new_.
std::string
replace(
    std::string const & value, std::string const & old, std::string const & new_);

} // namespace dopamine

#endif // _da25cc04_b8bb_4e69_8cd2_b27f5acf27fc
