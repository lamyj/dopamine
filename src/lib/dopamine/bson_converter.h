/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _c31aa1cf_ad8a_44f1_9d23_97c045ad81fd
#define _c31aa1cf_ad8a_44f1_9d23_97c045ad81fd

#include <mongo/bson/bson.h>
#include <odil/DataSet.h>
#include <odil/Value.h>

namespace dopamine
{

/// @brief Convert a data set to its BSON representation.
mongo::BSONObj as_bson(
    odil::DataSet const & data_set,
    odil::Value::Strings const & specific_character_set = {});

/// @brief Create a data set from its BSON representation.
odil::DataSet as_dataset(
    mongo::BSONObj const & bson,
    odil::Value::Strings const & specific_character_set = {});
}

#endif // _c31aa1cf_ad8a_44f1_9d23_97c045ad81fd
