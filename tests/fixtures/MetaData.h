/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _c1f2925f_2df1_438d_a45a_6a1d25f0bd51
#define _c1f2925f_2df1_438d_a45a_6a1d25f0bd51

#include <odil/DataSet.h>

#include "fixtures/Authorization.h"

namespace fixtures
{

/// @brief Fill the database with data-sets (meta-data only, no bulk data)
class MetaData: public fixtures::Authorization
{
public:
    MetaData();
    virtual ~MetaData();

    /// @brief Ordering for data sets.
    static bool less(odil::DataSet const & x, odil::DataSet const & y);
private:
    void _populate();
};

} // namespace fixtures

#endif // _c1f2925f_2df1_438d_a45a_6a1d25f0bd51
