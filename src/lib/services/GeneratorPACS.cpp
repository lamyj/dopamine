/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <odil/message/Response.h>

#include "core/ConfigurationPACS.h"
#include "core/LoggerPACS.h"
#include "GeneratorPACS.h"

namespace dopamine
{

namespace services
{

GeneratorPACS
{
    {
    }
}

GeneratorPACS
{
}

void
GeneratorPACS
{
    {
    }
    else
    {

GeneratorPACS
::GeneratorPACS():
    Generator(), _connection(NULL), _isconnected(false), _username(""),
    _query_retrieve_level(""), _instance_count_tags({}), _include_fields({}),
    _maximum_results(0), _skipped_results(0)
{
    // Nothing else.
}

GeneratorPACS
::~GeneratorPACS()
{
    if (this->_connection != NULL)
    {
        this->_cursor.release();
        delete this->_connection;
    }
}

odil::Value::Integer
GeneratorPACS
::initialize(odil::Association const & association,
             odil::message::Message const & message)
{
    // Get user identity
    this->_username =
        association.get_parameters().get_user_identity().primary_field;

    // Everything ok
    return odil::message::Response::Success;
}

bool
GeneratorPACS
::done() const
{
    if (this->_cursor == NULL)
    {
        return true;
    }

    return !this->_cursor->more();
}

odil::Value::Integer
GeneratorPACS
::initialize(mongo::BSONObj const & request)
{
    // Get configuration for Database connection
    MongoDBInformation db_information;
    std::string db_host = "";
    int db_port = -1;
    std::vector<std::string> indexeslist;
    ConfigurationPACS::get_instance().get_database_configuration(db_information,
                                                                 db_host,
                                                                 db_port,
                                                                 indexeslist);

    // Create connection with Database
    this->_connection = new MongoDBConnection(db_information, db_host, db_port,
                                              indexeslist);

    // Try to connect
    this->_isconnected = this->_connection->connect();
    if (this->_isconnected == false)
    {
        return odil::message::Response::ProcessingFailure;
    }

    // Everything ok
    return odil::message::Response::Success;
}

std::pair<std::string, int>
GeneratorPACS
::get_peer_information(std::string const & ae_title)
{
    return this->_connection->get_peer_information(ae_title);
}

void
GeneratorPACS
::set_username(std::string const & name)
{
    this->_username = name;
}

std::string
GeneratorPACS
::get_username() const
{
    return this->_username;
}

bool
GeneratorPACS
::is_connected() const
{
    return this->_isconnected;
}

void
GeneratorPACS
::set_query_retrieve_level(std::string const & query_retrieve_level)
{
    this->_query_retrieve_level = query_retrieve_level;
}

std::string
GeneratorPACS
::get_query_retrieve_level() const
{
    return this->_query_retrieve_level;
}

std::vector<std::string>
GeneratorPACS
::get_instance_count_tags() const
{
    return this->_instance_count_tags;
}

void
GeneratorPACS
::set_include_fields(std::vector<std::string> const & include_fields)
{
    this->_include_fields = include_fields;
}

std::vector<std::string> &
GeneratorPACS
::get_include_fields()
{
    return this->_include_fields;
}

void
GeneratorPACS
::set_maximum_results(int maximum_results)
{
    this->_maximum_results = maximum_results;
}

int
GeneratorPACS
::get_maximum_results() const
{
    return this->_maximum_results;
}

void
GeneratorPACS
::set_skipped_results(int skipped_results)
{
    this->_skipped_results = skipped_results;
}

int
GeneratorPACS
::get_skipped_results() const
{
    return this->_skipped_results;
}

bool
GeneratorPACS
::extract_query_retrieve_level(mongo::BSONObj const & mongo_object)
{
    // Query retrieve level should be present
    if (!mongo_object.hasField("00080052"))
    {
        logger_warning() << "Cannot find field QueryRetrieveLevel";
        return false;
    }

    // Read the Query Retrieve Level
    mongo::BSONObj const field_00080052 =
            mongo_object.getField("00080052").Obj();
    this->_query_retrieve_level =
            field_00080052.getField("Value").Array()[0].String();

    return true;
}

odil::Value::Integer
GeneratorPACS
::_get_count(std::string const & relatedElement,
             std::string const & ofElement,
             std::string const & value)
{
    mongo::BSONObj const object = BSON("distinct" << "datasets" <<
                                       "key" << relatedElement <<
                                       "query" << BSON(ofElement << value));

    mongo::BSONObj info;
    bool ret = this->_connection->run_command(object, info);
    if (!ret)
    {
        // error
    }

    return info["values"].Array().size();
}

odil::Element
GeneratorPACS
::compute_attribute(odil::Tag const & tag, odil::VR const & vr,
                    std::string const & value)
{
    if (tag == odil::registry::InstanceAvailability) // Instance Availability
    {
        return odil::Element({"ONLINE"}, vr);
    }
    else if (tag == odil::registry::ModalitiesInStudy) // Modalities in Study
    {
        mongo::BSONObj const object = BSON("distinct" << "datasets" <<
                                           "key" << "00080060.Value" <<
                                           "query" << BSON("0020000d.Value" <<
                                                           value));

        mongo::BSONObj info;
        bool ret = this->_connection->run_command(object, info);
        if (!ret)
        {
            // error
        }

        odil::Value::Strings values;
        for (auto const item : info.getField("values").Array())
        {
            values.push_back(item.String());
        }

        return odil::Element(values, vr);
    }
    else if (tag == "00201200") // Number of Patient Related Study
    {
        auto size = this->_get_count("0020000d", "00100020.Value", value);
        return odil::Element({size}, vr);
    }
    else if (tag == "00201202") // Number of Patient Related Series
    {
        auto size = this->_get_count("0020000e", "00100020.Value", value);
        return odil::Element({size}, vr);
    }
    else if (tag == "00201204") // Number of Patient Related Instances
    {
        auto size = this->_get_count("00080018", "00100020.Value", value);
        return odil::Element({size}, vr);
    }
    else if (tag == "00201206") // Number of Study Related Series
    {
        auto size = this->_get_count("0020000e", "0020000d.Value", value);
        return odil::Element({size}, vr);
    }
    else if (tag == "00201208") // Number of Study Related Instances
    {
        auto size = this->_get_count("00080018", "0020000d.Value", value);
        return odil::Element({size}, vr);
    }
    else if (tag == "00201209") // Number of Series Related Instances
    {
        auto size = this->_get_count("00080018", "0020000e.Value", value);
        return odil::Element({size}, vr);
    }

    return odil::Element();
}

} // namespace services

} // namespace dopamine
