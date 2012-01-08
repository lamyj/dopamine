#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestUser
#include <boost/test/unit_test.hpp>

#include "user.h"

#include <stdexcept>
#include <string>

#include <mongo/client/dbclient.h>

BOOST_AUTO_TEST_CASE(empty)
{
    User empty;
    BOOST_REQUIRE_EQUAL(empty.get_id(), "");
    BOOST_REQUIRE_EQUAL(empty.get_name(), "");
}

BOOST_AUTO_TEST_CASE(id_only)
{
    User id_only("foo");
    BOOST_REQUIRE_EQUAL(id_only.get_id(), "foo");
    BOOST_REQUIRE_EQUAL(id_only.get_name(), "");
}

BOOST_AUTO_TEST_CASE(id_and_name)
{
    User id_and_name("foo", "bar");
    BOOST_REQUIRE_EQUAL(id_and_name.get_id(), "foo");
    BOOST_REQUIRE_EQUAL(id_and_name.get_name(), "bar");
}

BOOST_AUTO_TEST_CASE(setters)
{
    User user;
    
    user.set_id("foo");
    BOOST_REQUIRE_EQUAL(user.get_id(), "foo");
    BOOST_REQUIRE_EQUAL(user.get_name(), "");
    
    user.set_name("bar");
    BOOST_REQUIRE_EQUAL(user.get_id(), "foo");
    BOOST_REQUIRE_EQUAL(user.get_name(), "bar");
}

BOOST_AUTO_TEST_CASE(ToBSON)
{
    User user("foo", "bar");
    mongo::BSONObj const object = user.to_bson();
    BOOST_REQUIRE_EQUAL(object.jsonString(), "{ \"id\" : \"foo\", \"name\" : \"bar\" }");
}

BOOST_AUTO_TEST_CASE(FromBSON_Empty)
{
    mongo::BSONObj object;
    User user;
    
    BOOST_REQUIRE_THROW(user.from_bson(object), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(FromBSON_Id)
{
    mongo::BSONObj object(BSON("id" << "foo"));
    User user;
    
    user.from_bson(object);
    BOOST_REQUIRE_EQUAL(user.get_id(), "foo");
    BOOST_REQUIRE_EQUAL(user.get_name(), "");
}

BOOST_AUTO_TEST_CASE(FromBSON_Name)
{
    mongo::BSONObj object(BSON("name" << "bar"));
    User user;
    
    BOOST_REQUIRE_THROW(user.from_bson(object), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(FromBSON_Id_and_Name)
{
    mongo::BSONObj object(BSON("id" << "foo" << "name" << "bar"));
    User user;
    
    user.from_bson(object);
    BOOST_REQUIRE_EQUAL(user.get_id(), "foo");
    BOOST_REQUIRE_EQUAL(user.get_name(), "bar");
}