/** @file ffs_dss_start_provisioning_session_request.c
 *
 * @brief DSS "start provisioning session" request implementation.
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "ffs/common/ffs_check_result.h"
#include "ffs/dss/model/ffs_dss_start_provisioning_session_request.h"

#define JSON_KEY_NONCE      "nonce"

/*
 * Serialize a DSS "start provisioning session" request.
 */
FFS_RESULT ffsDssSerializeStartProvisioningSessionRequest(
        FfsDssStartProvisioningSessionRequest_t *startProvisioningSessionRequest,
        FfsStream_t *outputStream)
{
    // Start the "start provisioning session" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the nonce field.
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_NONCE, startProvisioningSessionRequest->nonce,
            outputStream));

    // End the "start provisioning session" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    return FFS_SUCCESS;
}

/*
 * Deserialize a DSS "start provisioning session" request.
 */
FFS_RESULT ffsDssDeserializeStartProvisioningSessionRequest(
        FfsJsonValue_t *startProvisioningSessionRequestValue,
        FfsDssStartProvisioningSessionRequest_t *startProvisioningSessionRequest)
{
    (void) startProvisioningSessionRequestValue;
    (void) startProvisioningSessionRequest;

    return FFS_NOT_IMPLEMENTED;
}
