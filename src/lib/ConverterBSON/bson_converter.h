/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _8e0d86d9_dc96_475a_9ca8_42140db07a64
#define _8e0d86d9_dc96_475a_9ca8_42140db07a64

#include <memory>

#include <odil/DataSet.h>

#include <mongo/bson/bson.h>

#include "Condition.h"

namespace dopamine
{

struct FilterAction
{
    enum Type
    {
        INCLUDE=0,
        EXCLUDE,
        UNKNOWN
    };
};

typedef std::pair<std::shared_ptr<Condition>, FilterAction::Type> Filter;
typedef std::vector<Filter> Filters;


/// @brief Convert a data set to its BSON representation.
mongo::BSONObj as_bson(odil::DataSet const & data_set,
                       FilterAction::Type default_filter = FilterAction::UNKNOWN,
                       Filters const & filters = {},
                       odil::Value::Strings const & specific_character_set = {});

/// @brief Create a data set from its BSON representation.
odil::DataSet as_dataset(mongo::BSONObj const & bson);

} // namespace dopamine

#endif // _8e0d86d9_dc96_475a_9ca8_42140db07a64
