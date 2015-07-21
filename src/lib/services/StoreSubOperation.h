/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _783b07e2_38f1_4021_a423_6fe8d0934681
#define _783b07e2_38f1_4021_a423_6fe8d0934681

/* make sure OS specific configuration is included first */
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>

namespace dopamine
{

namespace services
{

/**
 * @brief \class The StoreSubOperation class
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

    /**
     * @brief Create an other association
     * @param destination_aetitle
     * @return EC_Normal if successful, an error code otherwise
     */
    OFCondition build_sub_association(DIC_AE destination_aetitle);

    /**
     * @brief call the store operation
     * @param dataset: Dataset to Store
     * @param priority: Priority of the operation
     * @return EC_Normal if successful, an error code otherwise
     */
    OFCondition perform_sub_operation(DcmDataset* dataset,
                                      T_DIMSE_Priority priority);

protected:

private:
    /// Current Network
    T_ASC_Network * _network;

    /// Requested association
    T_ASC_Association * _request_association;

    /// Response association
    T_ASC_Association * _response_association;

    /// original AE Title
    DIC_AE _original_aetitle;

    /// Original message ID
    DIC_US _original_message_id;

    /// flag to indicate if it is a new or current association
    bool _new_association;

    /**
     * @brief Add the presentation context
     * @param params: association parameters
     * @return EC_Normal if successful, an error code otherwise
     */
    OFCondition _add_all_storage_presentation_context(T_ASC_Parameters* params);

};

} // namespace services

} // namespace dopamine

#endif // _783b07e2_38f1_4021_a423_6fe8d0934681
