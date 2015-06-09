/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _96def0d5_038e_4a91_ae52_a1b223479305
#define _96def0d5_038e_4a91_ae52_a1b223479305

#include <string>

namespace dopamine
{

namespace ConverterBase64
{

unsigned int const DEFAULT_LINEBREAK = 72;
unsigned int const NO_LINEBREAK = 0;

std::string encode(std::string const & bindata, unsigned int linebreak = NO_LINEBREAK);

std::string decode(std::string const & ascdata);

} // namespace ConverterBase64

} // namespace dopamine

#endif // _96def0d5_038e_4a91_ae52_a1b223479305
