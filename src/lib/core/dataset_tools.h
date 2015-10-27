/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _156d9663_7ee7_402e_9829_4a1a2eb82c43
#define _156d9663_7ee7_402e_9829_4a1a2eb82c43

#include <string>

#include <dcmtkpp/DataSet.h>

namespace dopamine
{

std::string dataset_to_json_string(dcmtkpp::DataSet const & data_set);

std::string dataset_to_xml_string(dcmtkpp::DataSet const & data_set);

} // namespace dopamine

#endif // _156d9663_7ee7_402e_9829_4a1a2eb82c43
