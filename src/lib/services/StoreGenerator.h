/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _137519da_5031_4188_b52f_b1a3616696c5
#define _137519da_5031_4188_b52f_b1a3616696c5

#include "services/Generator.h"

namespace dopamine
{

namespace services
{

/**
 * @brief \class The StoreGenerator class
 */
class StoreGenerator : public Generator
{
public:
    /**
     * @brief Create an instance of StoreGenerator
     * @param username
     */
    StoreGenerator(std::string const & username);

    /// Destroy the store response generator
    virtual ~StoreGenerator();

    /**
     * @brief do the store operation
     * @return Status of the operation
     */
    virtual Uint16 process();

    /**
     * @brief Set the attribute _calling_aptitle
     * @param callingaptitle: new value
     */
    void set_calling_aptitle(std::string const & callingaptitle);

    /**
     * @brief Get value of attribute _calling_aptitle
     * @return _calling_aptitle
     */
    std::string get_calling_aptitle() const;

private:
    ///
    std::string _destination_path;

    ///
    std::string _calling_aptitle;

};

} // namespace services

} // namespace dopamine

#endif // _137519da_5031_4188_b52f_b1a3616696c5
