/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _137519da_5031_4188_b52f_b1a3616696c5
#define _137519da_5031_4188_b52f_b1a3616696c5

#include "services/ResponseGenerator.h"

namespace dopamine
{

namespace services
{

class StoreResponseGenerator : public ResponseGenerator
{
public:
    StoreResponseGenerator(std::string const & username);

    /// Destroy the store response generator
    virtual ~StoreResponseGenerator();

    virtual Uint16 set_query(DcmDataset * dataset);

    void set_callingaptitle(std::string const & callingaptitle);

private:
    std::string _destination_path;

    std::string _callingaptitle;

    void create_destination_path(DcmDataset *dataset);

    bool is_dataset_allowed_for_storage(mongo::BSONObj const & dataset);

};

} // namespace services

} // namespace dopamine

#endif // _137519da_5031_4188_b52f_b1a3616696c5
