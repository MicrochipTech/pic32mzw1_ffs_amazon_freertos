/** @file ffs_dss_start_pin_based_setup_response.c
 *
 * @brief DSS "start PIN-based setup" response.
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
#include "ffs/common/ffs_json.h"
#include "ffs/dss/model/ffs_dss_start_pin_based_setup_response.h"

#define JSON_KEY_NONCE              "nonce"
#define JSON_KEY_SESSION_ID         "sessionId"
#define JSON_KEY_CAN_PROCEED        "canProceed"

/*
 * Serialize a DSS "start PIN-based setup" response.
 */
FFS_RESULT ffsDssSerializeStartPinBasedSetupResponse(
        FfsDssStartPinBasedSetupResponse_t *startPinBasedSetupResponse,
        FfsStream_t *outputStream)
{
    (void) startPinBasedSetupResponse;
    (void) outputStream;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Deserialize a DSS "start PIN-based setup" response.
 */
FFS_RESULT ffsDssDeserializeStartPinBasedSetupResponse(
        FfsJsonValue_t *startPinBasedSetupResponseValue,
        FfsDssStartPinBasedSetupResponse_t *startPinBasedSetupResponse)
{
    // Zero out the "start PIN-based setup" response.
    memset(startPinBasedSetupResponse, 0, sizeof(*startPinBasedSetupResponse));

    // Parse {"nonce":"...","canProceed":"..."}.
    FfsJsonField_t nonceField = ffsCreateJsonField(JSON_KEY_NONCE, FFS_JSON_STRING);
    FfsJsonField_t canProceedField = ffsCreateJsonField(JSON_KEY_CAN_PROCEED, FFS_JSON_BOOLEAN);
    FfsJsonField_t *rootExpectedFields[] = { &nonceField, &canProceedField, NULL };

    FFS_CHECK_RESULT(ffsParseJsonObject(startPinBasedSetupResponseValue, rootExpectedFields));

    // Parse the nonce.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&nonceField.value,
            &startPinBasedSetupResponse->nonce));

    // Parse the 'canProceed' value.
    FFS_CHECK_RESULT(ffsParseJsonBoolean(&canProceedField.value, &startPinBasedSetupResponse->canProceed));

    return FFS_SUCCESS;
}

