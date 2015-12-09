/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _abef9025_9f25_4a3f_bc82_695c873ee02a
#define _abef9025_9f25_4a3f_bc82_695c873ee02a

#include "services/GeneratorPACS.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class Response Generator for C-GET services.
 */
class GetGenerator : public GeneratorPACS
{
public:
    typedef GetGenerator Self;
    typedef boost::shared_ptr<Self> Pointer;

    /// Create pointer to new instance of GetGenerator
    static Pointer New();

    /// Destroy the get response generator
    virtual ~GetGenerator();

    virtual dcmtkpp::Value::Integer initialize(dcmtkpp::Association const & association,
                                               dcmtkpp::message::Message const & message);

    virtual dcmtkpp::Value::Integer next();

    virtual dcmtkpp::Value::Integer initialize(mongo::BSONObj const & request);

    void set_query_retrieve_level(std::string const & query_retrieve_level);

    std::string get_query_retrieve_level() const;

    std::vector<std::string> get_instance_count_tags() const;

    void set_include_fields(std::vector<std::string> const & include_fields);

    std::vector<std::string>& get_include_fields();

    void set_maximum_results(int maximum_results);

    int get_maximum_results() const;

    void set_skipped_results(int skipped_results);

    int get_skipped_results() const;

protected:
    /// Create a default get response generator
    GetGenerator();

private:
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

    std::pair<dcmtkpp::DataSet, dcmtkpp::DataSet> _retrieve_dataset(mongo::BSONObj const & object);

};

} // namespace services

} // namespace dopamine

#endif // _abef9025_9f25_4a3f_bc82_695c873ee02a
