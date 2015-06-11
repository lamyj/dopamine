/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _76b2ab31_bb60_4366_ad0b_55c6eb286fdf
#define _76b2ab31_bb60_4366_ad0b_55c6eb286fdf

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

    virtual Uint16 process();

    std::string retrieve_dataset_as_string(mongo::BSONObj const & object);

    DcmDataset* retrieve_dataset(mongo::BSONObj const & object);

    std::vector<std::string> get_instance_count_tags() const;

    bool get_convert_modalities_in_study() const;

    std::string get_query_retrieve_level() const;

    void set_include_fields(std::vector<std::string> include_fields);

    void set_maximum_results(int maximum_results);

    int get_maximum_results() const;

    void set_skipped_results(int skipped_results);

    int get_skipped_results() const;

    void set_fuzzy_matching(bool fuzzy_matching);

    bool get_fuzzy_matching() const;

    mongo::BSONObj compute_attribute(std::string const & attribute,
                                     std::string const & value);

protected:
    std::string _service_name;

    std::string _query_retrieve_level;

    std::vector<std::string> _instance_count_tags;

    /// flag indicating if modalities should be convert
    bool _convert_modalities_in_study;

private:
    std::vector<std::string> _include_fields;

    int _maximum_results;
    int _skipped_results;
    bool _fuzzy_matching;

    unsigned int _get_count(std::string const & relatedElement,
                            std::string const & ofElement,
                            std::string const & value);

};

} // namespace services

} // namespace dopamine

#endif // _76b2ab31_bb60_4366_ad0b_55c6eb286fdf
