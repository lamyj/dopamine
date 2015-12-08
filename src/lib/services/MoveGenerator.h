/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _67fe6116_89f6_4b1c_b2b4_8046e63b1537
#define _67fe6116_89f6_4b1c_b2b4_8046e63b1537

#include "services/GeneratorPACS.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class Response Generator for C-MOVE services.
 */
class MoveGenerator : public GeneratorPACS
{
public:
    typedef MoveGenerator Self;
    typedef boost::shared_ptr<Self> Pointer;

    /// Create pointer to new instance of MoveGenerator
    static Pointer New();

    /// Destroy the move response generator
    virtual ~MoveGenerator();

    virtual dcmtkpp::Value::Integer initialize(dcmtkpp::Association const & association,
                                               dcmtkpp::message::Message const & message);

    virtual dcmtkpp::Value::Integer next();

protected:
    /// Create a default move response generator
    MoveGenerator();

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

#endif // _67fe6116_89f6_4b1c_b2b4_8046e63b1537
