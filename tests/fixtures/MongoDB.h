/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _cfaede7e_8acc_47ab_a740_9b46f12e0058
#define _cfaede7e_8acc_47ab_a740_9b46f12e0058

#include <string>

#include <mongo/client/dbclient.h>

namespace fixtures
{

/**
 * @brief Create a MongoDB database with a random name and delete it at
 * destruction.
 */
class MongoDB
{
public:
    mongo::DBClientConnection connection;
    std::string const database;

    MongoDB();

    virtual ~MongoDB() =0;
private:
    static unsigned long const _seed;

    static unsigned long _get_seed();
    std::string _generate_database_name() const;
};

} // namespace fixtures

#endif // _cfaede7e_8acc_47ab_a740_9b46f12e0058
