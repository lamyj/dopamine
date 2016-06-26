/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _d11a0665_fab9_4ad0_8287_043e7617d0f1
#define _d11a0665_fab9_4ad0_8287_043e7617d0f1

#include <mongo/client/dbclient.h>

#include "dbconnection/MongoDBConnection.h"
#include "services/Generator.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class The Generator class
 * Base class for all response generator
 */
class GeneratorPACS : public Generator
{
public:
    typedef GeneratorPACS Self;
    typedef boost::shared_ptr<Self> Pointer;


    /// @brief Default Constructor for GeneratorPACS
    GeneratorPACS();

    /// Destroy the instance of GeneratorPACS
    virtual ~GeneratorPACS();

    virtual odil::Value::Integer initialize(
            odil::Association const & association,
            odil::message::Message const & message);

    virtual bool done() const;

    virtual odil::Value::Integer initialize(mongo::BSONObj const & request);

    virtual std::pair<std::string, int> get_peer_information(std::string const & ae_title);

    void set_username(std::string const & name);

    std::string get_username() const;

    bool is_connected() const;

    void set_query_retrieve_level(std::string const & query_retrieve_level);

    std::string get_query_retrieve_level() const;

    std::vector<std::string> get_instance_count_tags() const;

    void set_include_fields(std::vector<std::string> const & include_fields);

    std::vector<std::string>& get_include_fields();

    void set_maximum_results(int maximum_results);

    int get_maximum_results() const;

    void set_skipped_results(int skipped_results);

    int get_skipped_results() const;

    odil::Element compute_attribute(odil::Tag const & tag,
                                       odil::VR const & vr,
                                       std::string const & value);

protected:
    bool extract_query_retrieve_level(mongo::BSONObj const & mongo_object);

    /// Connection to the Database
    MongoDBConnection * _connection;

    /// Flag indicating if connection is established
    bool _isconnected;

    /// User name
    std::string _username;

    /// Cursor to the database
    mongo::unique_ptr<mongo::DBClientCursor> _cursor;

    /// QueryRetrieveLevel
    std::string _query_retrieve_level;

    ///
    std::vector<std::string> _instance_count_tags;

    /// Fields to retrieve
    std::vector<std::string> _include_fields;

    /// Maximum number of dataset to retrieve
    int _maximum_results;

    /// Number of response to ignore
    int _skipped_results;

private:
    odil::Value::Integer _get_count(std::string const & relatedElement,
                                       std::string const & ofElement,
                                       std::string const & value);

};

} // namespace services

} // namespace dopamine

#endif // _d11a0665_fab9_4ad0_8287_043e7617d0f1
