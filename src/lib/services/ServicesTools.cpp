/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/registry.h>

#include "ServicesTools.h"

namespace dopamine
{

namespace services
{

dcmtkpp::DataSet
create_status_detail(Uint16 const errorCode,
                     dcmtkpp::Tag const & key,
                     std::string const & comment)
{
    dcmtkpp::DataSet data_set;
    data_set.add(dcmtkpp::registry::Status,
                 dcmtkpp::Element({errorCode}, dcmtkpp::VR::US));
    data_set.add(dcmtkpp::registry::OffendingElement,
                 dcmtkpp::Element({std::string(key)}, dcmtkpp::VR::AT));
    data_set.add(dcmtkpp::registry::ErrorComment,
                 dcmtkpp::Element({comment}, dcmtkpp::VR::LO));

    return data_set;
}

std::string
replace(std::string const & value, std::string const & old,
        std::string const & new_)
{
    std::string result(value);
    size_t begin=0;
    while(std::string::npos != (begin=result.find(old, begin)))
    {
        result = result.replace(begin, old.size(), new_);
        begin = (begin+new_.size()<result.size())?begin+new_.size()
                                                 :std::string::npos;
    }

    return result;
}

} // namespace services

} // namespace dopamine
