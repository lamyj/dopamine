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

class QueryRetrieveGenerator : public Generator
{
public:
    QueryRetrieveGenerator(std::string const & username,
                           std::string const & service_name);

    virtual Uint16 process();

    std::string retrieve_dataset_as_string(mongo::BSONObj const & object);

    DcmDataset* retrieve_dataset(mongo::BSONObj const & object);

    std::vector<std::string> get_instance_count_tags() const;

    bool get_convert_modalities_in_study() const;

    std::string get_query_retrieve_level() const;

    void set_includefields(std::vector<std::string> includefields);

    void set_maximumResults(int maximumResults);

    int get_maximumResults() const;

    void set_skippedResults(int skippedResults);

    int get_skippedResults() const;

    void set_fuzzymatching(bool fuzzymatching);

    bool get_fuzzymatching() const;

    mongo::BSONObj compute_attribute(std::string const & attribute,
                                     std::string const & value);

protected:
    std::string _service_name;

    std::string _query_retrieve_level;

    std::vector<std::string> _instance_count_tags;

    /// flag indicating if modalities should be convert
    bool _convert_modalities_in_study;

private:
    std::vector<std::string> _includefields;

    int _maximumResults;
    int _skippedResults;
    bool _fuzzymatching;

    unsigned int get_count(const std::string &relatedElement,
                           const std::string &ofElement,
                           const std::string &value);

};

} // namespace services

} // namespace dopamine

#endif // _76b2ab31_bb60_4366_ad0b_55c6eb286fdf
