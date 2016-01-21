/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _28858a50_3fa5_495e_a532_952cca4cbe5c
#define _28858a50_3fa5_495e_a532_952cca4cbe5c

#include <string>
#include <vector>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmnet/assoc.h>

namespace dopamine
{

namespace services
{

/// @brief Presentation con
struct PresentationContext
{
    std::string abstract_syntax;
    std::vector<std::string> transfer_syntaxes;
    T_ASC_SC_ROLE role;
};

}

}

#endif // _28858a50_3fa5_495e_a532_952cca4cbe5c
