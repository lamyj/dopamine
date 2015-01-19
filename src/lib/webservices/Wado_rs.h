/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _23d72937_8451_4cd1_a64d_77b774672145
#define _23d72937_8451_4cd1_a64d_77b774672145

#include <string>

namespace dopamine
{

namespace webservices
{

const std::string MIME_TYPE_DICOM = "application/dicom";

std::string wado_rs(std::string const & pathinfo, std::string & filename);

} // namespace webservices

} // namespace dopamine

#endif // _23d72937_8451_4cd1_a64d_77b774672145
