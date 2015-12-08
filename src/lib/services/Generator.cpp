/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "Generator.h"

namespace dopamine
{

namespace services
{

Generator
::Generator():
    _meta_information(dcmtkpp::DataSet()), _current_dataset(dcmtkpp::DataSet())
{
    // Nothing to do.
}

Generator
::~Generator()
{
    // Nothing to do.
}

std::pair<dcmtkpp::DataSet, dcmtkpp::DataSet> Generator::get() const
{
    return std::make_pair(this->_meta_information, this->_current_dataset);
}

} // namespace services

} // namespace dopamine
