/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _3b57890b_d730_458f_8bce_37bd2d189a9b
#define _3b57890b_d730_458f_8bce_37bd2d189a9b

#include <string>

#include <dcmtkpp/DataSet.h>
#include <dcmtkpp/Tag.h>

namespace dopamine
{

namespace services
{

/**
 * @brief Create the status detail Dataset
 * @param errorCode: Error code
 * @param key: Tag in error
 * @param comment: error detail
 * @return the status detail Dataset
 */
dcmtkpp::DataSet create_status_detail(Uint16 const errorCode,
                                      dcmtkpp::Tag const & key,
                                      std::string const & comment);

/**
 * Replace all given pattern by another
 * @param value: given input string
 * @param old: search pattern to be replaced
 * @param new_: new pattern
 * @return string
 */
std::string replace(std::string const & value,
                    std::string const & old,
                    std::string const & new_);

} // namespace services

} // namespace dopamine

#endif // _3b57890b_d730_458f_8bce_37bd2d189a9b
