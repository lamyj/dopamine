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
#include <odil/AssociationParameters.h>

namespace dopamine
{

/// @brief Replace every occurence of old by new_.
std::string
replace(
    std::string const & value, std::string const & old, std::string const & new_);

/// @brief Return the name of the principal of a DICOM association.
std::string get_principal(odil::AssociationParameters const & parameters);

/// @brief Return the UID name if it is known.
std::string get_uid_name(std::string const & uid);

} // namespace dopamine

#endif // _da25cc04_b8bb_4e69_8cd2_b27f5acf27fc
