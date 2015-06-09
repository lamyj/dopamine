/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _137519da_5031_4188_b52f_b1a3616696c5
#define _137519da_5031_4188_b52f_b1a3616696c5

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dcdatset.h>

#include "services/Generator.h"

namespace dopamine
{

namespace services
{

/**
 * @brief The StoreGenerator class
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

    virtual Uint16 process();

    void set_calling_aptitle(std::string const & callingaptitle);

    std::string get_calling_aptitle() const;

private:
    std::string _destination_path;

    std::string _calling_aptitle;

};

} // namespace services

} // namespace dopamine

#endif // _137519da_5031_4188_b52f_b1a3616696c5
