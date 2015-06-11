/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _67fe6116_89f6_4b1c_b2b4_8046e63b1537
#define _67fe6116_89f6_4b1c_b2b4_8046e63b1537

#include "QueryRetrieveGenerator.h"
#include "services/StoreSubOperation.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class The RetrieveGenerator class
 */
class RetrieveGenerator : public QueryRetrieveGenerator
{
public:
    /**
     * @brief Create an instance of RetrieveGenerator
     * @param username
     */
    RetrieveGenerator(std::string const & username);

    /// Destroy the get response generator
    virtual ~RetrieveGenerator();

protected:

private:

};

/**
 * @brief The RetrieveContext struct
 */
struct RetrieveContext
{
public:
    RetrieveContext(RetrieveGenerator * generator,
                    StoreSubOperation * storeprovider):
        _generator(generator), _storeprovider(storeprovider) {}

    RetrieveGenerator * get_generator() const
        { return this->_generator; }

    StoreSubOperation * get_storeprovider() const
        { return this->_storeprovider; }

private:
    RetrieveGenerator* _generator;
    StoreSubOperation* _storeprovider;

};

} // namespace services

} // namespace dopamine

#endif // _67fe6116_89f6_4b1c_b2b4_8046e63b1537
