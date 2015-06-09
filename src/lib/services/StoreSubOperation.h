/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _783b07e2_38f1_4021_a423_6fe8d0934681
#define _783b07e2_38f1_4021_a423_6fe8d0934681

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>

namespace dopamine
{

namespace services
{

/**
 * @brief The StoreSubOperation class
 */
class StoreSubOperation
{
public:
    /**
     * @brief Create an instance of StoreSubOperation
     * @param network
     * @param request_association
     * @param messageID
     */
    StoreSubOperation(T_ASC_Network * network,
                      T_ASC_Association * request_association,
                      DIC_US messageID);

    /// Destroy the instance of StoreSubOperation
    virtual ~StoreSubOperation();

    OFCondition build_sub_association(DIC_AE destination_aetitle);

    OFCondition perform_sub_operation(DcmDataset* dataset, T_DIMSE_Priority priority);

protected:

private:
    ///
    T_ASC_Network * _network;

    ///
    T_ASC_Association * _request_association;

    ///
    T_ASC_Association * _response_association;

    /// original AE Title
    DIC_AE _original_aetitle;

    DIC_US _original_message_id;

    bool _new_association;

    /**
     * Add the presentation context
     * @param params: association parameters
     * @return EC_Normal if successful, an error code otherwise
     */
    OFCondition _add_all_storage_presentation_context(T_ASC_Parameters* params);

};

} // namespace services

} // namespace dopamine

#endif // _783b07e2_38f1_4021_a423_6fe8d0934681
