/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _e153b729_6fe8_4b4f_939b_a9ed5e82d886
#define _e153b729_6fe8_4b4f_939b_a9ed5e82d886

#include "dopamine/AccessControlList.h"

#include "fixtures/MongoDB.h"

namespace fixtures
{

/**
 * @brief Create an authorization entry for each service, without constraints.
 */
class Authorization: public MongoDB
{
public:
    dopamine::AccessControlList acl;

    Authorization();
    virtual ~Authorization();
};

} // namespace fixtures

#endif // _e153b729_6fe8_4b4f_939b_a9ed5e82d886
