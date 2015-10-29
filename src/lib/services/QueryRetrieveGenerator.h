/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _76b2ab31_bb60_4366_ad0b_55c6eb286fdf
#define _76b2ab31_bb60_4366_ad0b_55c6eb286fdf

#include <dcmtkpp/DataSet.h>

#include "Generator.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class The QueryRetrieveGenerator class
 */
class QueryRetrieveGenerator : public Generator
{
public:
    /**
     * @brief Create an instance of QueryRetrieveGenerator
     * @param username
     * @param service_name
     */
    QueryRetrieveGenerator(std::string const & username,
                           std::string const & service_name);

    /// Destroy the instance of QueryRetrieveGenerator
    virtual ~QueryRetrieveGenerator();

    /**
     * @brief Execute Get or Move operation
     * @return Status of the operation
     */
    virtual Uint16 process();

    /**
     * @brief Retrieve a dataset from database
     * @param object: Query
     * @return dataset as string
     */
    std::string retrieve_dataset_as_string(mongo::BSONObj const & object);

    /**
     * @brief Retrieve a dataset from database
     * @param object: Query
     * @return dataset
     */
    dcmtkpp::DataSet retrieve_dataset(mongo::BSONObj const & object);

    /**
     * @brief Accessor for attribute _instance_count_tags
     * @return attribute _instance_count_tags
     */
    std::vector<std::string> get_instance_count_tags() const;

    /**
     * @brief Accessor for attribute _convert_modalities_in_study
     * @return attribute _convert_modalities_in_study
     */
    bool get_convert_modalities_in_study() const;

    /**
     * @brief Accessor for attribute _query_retrieve_level
     * @return attribute _query_retrieve_level
     */
    std::string get_query_retrieve_level() const;

    /**
     * @brief set the attribute _include_fields
     * @param include_fields: new value
     */
    void set_include_fields(std::vector<std::string> include_fields);

    /**
     * @brief set the attribute _maximum_results
     * @param maximum_results: new value
     */
    void set_maximum_results(int maximum_results);

    /**
     * @brief Accessor for attribute _maximum_results
     * @return attribute _maximum_results
     */
    int get_maximum_results() const;

    /**
     * @brief set the attribute _skipped_results
     * @param skipped_results: new value
     */
    void set_skipped_results(int skipped_results);

    /**
     * @brief Accessor for attribute _skipped_results
     * @return attribute _skipped_results
     */
    int get_skipped_results() const;

    /**
     * @brief set the attribute _fuzzy_matching
     * @param fuzzy_matching: new value
     */
    void set_fuzzy_matching(bool fuzzy_matching);

    /**
     * @brief Accessor for attribute _fuzzy_matching
     * @return attribute _fuzzy_matching
     */
    bool get_fuzzy_matching() const;

    /**
     * @brief Compute value for specific DICOM tag
     * @param attribute: attribute to compute
     * @param value: value of related attribute
     * @return
     */
    mongo::BSONObj compute_attribute(std::string const & attribute,
                                     std::string const & value);

protected:
    /// Service name (Query or Retrieve)
    std::string _service_name;

    /// QueryRetrieveLevel
    std::string _query_retrieve_level;

    ///
    std::vector<std::string> _instance_count_tags;

    /// flag indicating if modalities should be convert
    bool _convert_modalities_in_study;

private:
    /// Fields to retrieve
    std::vector<std::string> _include_fields;

    /// Maximum number of dataset to retrieve
    int _maximum_results;

    /// Number of response to ignore
    int _skipped_results;

    /// Flag to use fuzzy matching or not
    bool _fuzzy_matching;

    /**
     * @brief Get related element number of a given element
     * @param relatedElement: related DICOM tag
     * @param ofElement: given DICOM tag
     * @param value: value of the given tag
     * @return number of related element
     */
    unsigned int _get_count(std::string const & relatedElement,
                            std::string const & ofElement,
                            std::string const & value);

};

} // namespace services

} // namespace dopamine

#endif // _76b2ab31_bb60_4366_ad0b_55c6eb286fdf
