#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestProtocol
#include <boost/test/unit_test.hpp>

#include "protocol.h"

#include <stdexcept>
#include <string>

#include <mongo/client/dbclient.h>

BOOST_AUTO_TEST_CASE(empty)
{
    research_pacs::Protocol empty;
    BOOST_REQUIRE_EQUAL(empty.get_id(), "");
    BOOST_REQUIRE_EQUAL(empty.get_name(), "");
    BOOST_REQUIRE_EQUAL(empty.get_sponsor(), "");
}

BOOST_AUTO_TEST_CASE(ToBSON)
{
    research_pacs::Protocol protocol("id", "name", "sponsor");
    mongo::BSONObj const object = protocol.to_bson();
    BOOST_REQUIRE_EQUAL(object.jsonString(), 
        "{ \"id\" : \"id\", \"name\" : \"name\", \"sponsor\" : \"sponsor\" }");
}

BOOST_AUTO_TEST_CASE(FromBSON)
{
    mongo::BSONObj object(BSON("id" << "id" << "name" << "name" << "sponsor" << "sponsor"));
    research_pacs::Protocol protocol;
    
    protocol.from_bson(object);
    BOOST_REQUIRE_EQUAL(protocol.get_id(), "id");
    BOOST_REQUIRE_EQUAL(protocol.get_name(), "name");
    BOOST_REQUIRE_EQUAL(protocol.get_sponsor(), "sponsor");
}
