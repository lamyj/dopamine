/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _ee9915a2_a504_4b21_8d43_7938c66c526e
#define _ee9915a2_a504_4b21_8d43_7938c66c526e

#include <odil/AssociationParameters.h>
#include <odil/DataSet.h>
#include <odil/message/Request.h>

#include "services/QueryRetrieveGenerator.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class Response Generator for C-FIND services.
 */
class FindGenerator : public QueryRetrieveGenerator
{
public:
    /// @brief Constructor.
    FindGenerator(
        odil::AssociationParameters const & parameters,
        MongoDBConnection & db_connection);
    
    /// @brief Destructor.
    virtual ~FindGenerator();

    virtual void initialize(odil::message::Request const & request);
    virtual void next();

private:
    std::string _query_retrieve_level;
    /// @brief Flag indicating whether modalities in each study are required.
    bool _compute_modalities_in_study;
};

} // namespace services

} // namespace dopamine

#endif // _ee9915a2_a504_4b21_8d43_7938c66c526e
