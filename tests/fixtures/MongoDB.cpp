/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "MongoDB.h"

#include <cstdlib>
#include <string>
#include <sys/time.h>

#include <mongo/client/dbclient.h>

namespace fixtures
{

MongoDB
::MongoDB()
: database(this->_generate_database_name())
{
    this->connection.connect("localhost");
}

MongoDB
::~MongoDB()
{
    this->connection.dropDatabase(this->database);
}

unsigned long const
MongoDB
::_seed(MongoDB::_get_seed());

unsigned long
MongoDB
::_get_seed()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    unsigned long const seed(1000000*tv.tv_sec + tv.tv_usec);

    std::srand(seed);
    return seed;
}

std::string
MongoDB
::_generate_database_name() const
{
    std::string database;
    for(int i=0; i<20; ++i)
    {
        database += 'A'+int(std::rand()/float(RAND_MAX)*26.);
    }
    return database;
}

} // namespace fixtures
