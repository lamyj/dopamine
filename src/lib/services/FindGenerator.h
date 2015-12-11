/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _ee9915a2_a504_4b21_8d43_7938c66c526e
#define _ee9915a2_a504_4b21_8d43_7938c66c526e

#include "services/GeneratorPACS.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class Response Generator for C-FIND services.
 */
class FindGenerator : public GeneratorPACS
{
public:
    typedef FindGenerator Self;
    typedef boost::shared_ptr<Self> Pointer;

    /// Create pointer to new instance of FindGenerator
    static Pointer New();
    
    /// Destroy the find response generator
    virtual ~FindGenerator();

    virtual dcmtkpp::Value::Integer initialize(
            dcmtkpp::Association const & association,
            dcmtkpp::message::Message const & message);

    virtual dcmtkpp::Value::Integer next();

    virtual dcmtkpp::Value::Integer initialize(mongo::BSONObj const & request);

    void set_query_retrieve_level(std::string const & query_retrieve_level);

    std::string get_query_retrieve_level() const;

    std::vector<std::string> get_instance_count_tags() const;

    bool get_convert_modalities_in_study() const;

    void set_include_fields(std::vector<std::string> const & include_fields);

    std::vector<std::string>& get_include_fields();

    void set_maximum_results(int maximum_results);

    int get_maximum_results() const;

    void set_skipped_results(int skipped_results);

    int get_skipped_results() const;

    void set_fuzzy_matching(bool fuzzy_matching);

    bool get_fuzzy_matching() const;

protected:
    /// Create a default find response generator
    FindGenerator();

private:
    /// QueryRetrieveLevel
    std::string _query_retrieve_level;

    ///
    std::vector<std::string> _instance_count_tags;

    /// flag indicating if modalities should be convert
    bool _convert_modalities_in_study;

    /// Fields to retrieve
    std::vector<std::string> _include_fields;

    /// Maximum number of dataset to retrieve
    int _maximum_results;

    /// Number of response to ignore
    int _skipped_results;

    bool _fuzzy_matching;

    std::pair<dcmtkpp::DataSet, dcmtkpp::DataSet>
            _retrieve_dataset(mongo::BSONObj const & object);

};

} // namespace services

} // namespace dopamine

#endif // _ee9915a2_a504_4b21_8d43_7938c66c526e
