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
    StoreResponseGenerator(T_ASC_Association * request_association);

    /// Destroy the store response generator
    virtual ~StoreResponseGenerator();

    void process(T_DIMSE_StoreProgress *progress,    /* progress state */
                 T_DIMSE_C_StoreRQ *req,             /* original store request */
                 char *imageFileName,                /* being received into */
                 DcmDataset **imageDataSet,          /* being received into */
                 /* out */
                 T_DIMSE_C_StoreRSP *rsp,            /* final store response */
                 DcmDataset **stDetail);

    virtual Uint16 set_query(DcmDataset * dataset);

private:
    std::string _destination_path;

    void create_destination_path(DcmDataset *dataset);

    bool is_dataset_allowed_for_storage(mongo::BSONObj const & dataset);

};

} // namespace services

} // namespace dopamine

#endif // _137519da_5031_4188_b52f_b1a3616696c5
