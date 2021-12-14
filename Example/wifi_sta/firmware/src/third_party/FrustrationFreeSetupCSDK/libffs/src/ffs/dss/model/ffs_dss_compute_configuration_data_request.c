/** @file ffs_dss_compute_configuration_data_request.h
 *
 * @brief DSS "compute configuration data" request.
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
#include "ffs/dss/model/ffs_dss_compute_configuration_data_request.h"

#include <string.h>

#define JSON_KEY_NONCE              "nonce"
#define JSON_KEY_SESSION_ID         "sessionId"

/*
 * Serialize a DSS "compute configuration data" request.
 */
FFS_RESULT ffsDssSerializeComputeConfigurationDataRequest(
        FfsDssComputeConfigurationDataRequest_t *computeConfigurationDataRequest,
        FfsStream_t *outputStream)
{
    // Start the "compute configuration data" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the nonce field.
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_NONCE, computeConfigurationDataRequest->nonce,
            outputStream));

    // Serialize the session ID field.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_SESSION_ID,
            computeConfigurationDataRequest->sessionId, outputStream));

    // Serialize the "device details" field.
    FFS_CHECK_RESULT(ffsDssSerializeDeviceDetailsField(&computeConfigurationDataRequest->deviceDetails,
            outputStream));

    // End the "compute configuration data" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    return FFS_SUCCESS;
}

/*
 * Deserialize a DSS "compute configuration data" request.
 */
FFS_RESULT ffsDssDeserializeComputeConfigurationDataRequest(
        FfsJsonValue_t *computeConfigurationDataRequestValue,
        FfsDssComputeConfigurationDataRequest_t *computeConfigurationDataRequest,
        FfsJsonValue_t *configurationObjectValue)
{
    (void) computeConfigurationDataRequestValue;
    (void) computeConfigurationDataRequest;
    (void) configurationObjectValue;

    return FFS_NOT_IMPLEMENTED;
}
