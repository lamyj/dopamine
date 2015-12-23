/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleMongoDBInformation
#include <boost/test/unit_test.hpp>

#include "dbconnection/MongoDBInformation.h"

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_AUTO_TEST_CASE(Constructor)
{
    // Default constructor
    dopamine::MongoDBInformation * info = new dopamine::MongoDBInformation();
    BOOST_REQUIRE(info != NULL);
    delete info;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Copy Constructor
 */
BOOST_AUTO_TEST_CASE(CopyConstructor)
{
    dopamine::MongoDBInformation info;
    info.set_db_name("db_name");
    info.set_bulk_data("bulk_data");
    info.set_user("user");
    info.set_password("password");

    dopamine::MongoDBInformation infocopy(info);
    BOOST_CHECK_EQUAL(infocopy.get_db_name(), info.get_db_name());
    BOOST_CHECK_EQUAL(infocopy.get_bulk_data(), info.get_bulk_data());
    BOOST_CHECK_EQUAL(infocopy.get_user(), info.get_user());
    BOOST_CHECK_EQUAL(infocopy.get_password(), info.get_password());
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Accessors
 */
BOOST_AUTO_TEST_CASE(Accessors)
{
    dopamine::MongoDBInformation info;
    BOOST_CHECK_EQUAL(info.get_db_name(), "");
    BOOST_CHECK_EQUAL(info.get_bulk_data(), "");
    BOOST_CHECK_EQUAL(info.get_user(), "");
    BOOST_CHECK_EQUAL(info.get_password(), "");

    info.set_db_name("db_name");
    info.set_bulk_data("bulk_data");
    info.set_user("user");
    info.set_password("password");

    BOOST_CHECK_EQUAL(info.get_db_name(), "db_name");
    BOOST_CHECK_EQUAL(info.get_bulk_data(), "bulk_data");
    BOOST_CHECK_EQUAL(info.get_user(), "user");
    BOOST_CHECK_EQUAL(info.get_password(), "password");
}
