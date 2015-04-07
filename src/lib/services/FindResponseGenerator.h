/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _ee9915a2_a504_4b21_8d43_7938c66c526e
#define _ee9915a2_a504_4b21_8d43_7938c66c526e

#include "services/QueryRetrieveGenerator.h"

namespace dopamine
{

namespace services
{

/**
 * @brief Response Generator for C-FIND services.
 */
class FindResponseGenerator : public QueryRetrieveGenerator
{
public :
    /// Create a default find response generator
    FindResponseGenerator(std::string const & username);
    
    /// Destroy the find response generator
    virtual ~FindResponseGenerator();

protected:

private :
    
};

} // namespace services

} // namespace dopamine

#endif // _ee9915a2_a504_4b21_8d43_7938c66c526e
