/** @file ffs_dss_start_pin_based_setup_request.c
 *
 * @brief DSS "start PIN-based setup" request.
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
#include "ffs/dss/model/ffs_dss_start_pin_based_setup_request.h"

#define JSON_KEY_NONCE              "nonce"
#define JSON_KEY_SESSION_ID         "sessionId"
#define JSON_KEY_HASHED_PIN         "hashedPin"

/*
 * Serialize a DSS "start PIN-based setup" request.
 */
FFS_RESULT ffsDssSerializeStartPinBasedSetupRequest(
        FfsDssStartPinBasedSetupRequest_t *startPinBasedSetupRequest,
        FfsStream_t *outputStream)
{
    // Start the "start PIN-based setup" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the nonce field.
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_NONCE, startPinBasedSetupRequest->nonce,
            outputStream));

    // Serialize the session ID field.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_SESSION_ID, startPinBasedSetupRequest->sessionId,
            outputStream));

    // Serialize the "device details" field.
    FFS_CHECK_RESULT(ffsDssSerializeDeviceDetailsField(&startPinBasedSetupRequest->deviceDetails,
            outputStream));

    return FFS_SUCCESS;
}

FFS_RESULT ffsDssAddHashedPinToStartPinBasedSetupRequest(
        const char *hashedPin,
        FfsStream_t *outputStream)
{
    // Serialize the hashed PIN field.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_HASHED_PIN, hashedPin,
            outputStream));

    return FFS_SUCCESS;
}

/*
 * Complete serializing a DSS "start PIN-based setup" request.
 */
FFS_RESULT ffsDssFinishSerializingStartPinBasedSetupRequest(FfsStream_t *outputStream)
{
    // End the "start PIN-based setup" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    return FFS_SUCCESS;
}

/*
 * Deserialize a DSS "start PIN-based setup" request.
 */
FFS_RESULT ffsDssDeserializeStartPinBasedSetupRequest(
        FfsJsonValue_t *startPinBasedSetupRequestValue,
        FfsDssStartPinBasedSetupRequest_t *startPinBasedSetupRequest)
{
    (void) startPinBasedSetupRequestValue;
    (void) startPinBasedSetupRequest;

    return FFS_NOT_IMPLEMENTED;
}
