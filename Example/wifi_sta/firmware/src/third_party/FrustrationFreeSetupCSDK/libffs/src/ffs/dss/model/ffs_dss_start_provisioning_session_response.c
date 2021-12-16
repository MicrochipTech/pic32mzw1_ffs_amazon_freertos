/** @file ffs_dss_start_provisioning_session_response.c
 *
 * @brief DSS "start provisioning session" response implementation.
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
#include "ffs/dss/model/ffs_dss_start_provisioning_session_response.h"

#define JSON_KEY_NONCE          "nonce"
#define JSON_KEY_SESSION_ID     "sessionId"
#define JSON_KEY_CAN_PROCEED    "canProceed"
#define JSON_KEY_PIN_SALT       "salt"

/*
 * Serialize a DSS "start provisioning session" response.
 */
FFS_RESULT ffsDssSerializeStartProvisioningSessionResponse(
        FfsDssStartProvisioningSessionResponse_t *startProvisioningSessionResponse,
        FfsStream_t *outputStream)
{
    (void) startProvisioningSessionResponse;
    (void) outputStream;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Deserialize a DSS "start provisioning session" response.
 */
FFS_RESULT ffsDssDeserializeStartProvisioningSessionResponse(
        FfsJsonValue_t *startProvisioningSessionResponseValue,
        FfsDssStartProvisioningSessionResponse_t *startProvisioningSessionResponse)
{
    // Zero out the "start provisioning session" response.
    memset(startProvisioningSessionResponse, 0, sizeof(*startProvisioningSessionResponse));

    // Parse {"nonce":"...","sessionId":"...","canProceed":"...","salt":"..."}.
    FfsJsonField_t nonceField = ffsCreateJsonField(JSON_KEY_NONCE, FFS_JSON_STRING);
    FfsJsonField_t sessionIdField = ffsCreateJsonField(JSON_KEY_SESSION_ID, FFS_JSON_STRING);
    FfsJsonField_t canProceedField = ffsCreateJsonField(JSON_KEY_CAN_PROCEED, FFS_JSON_BOOLEAN);
    FfsJsonField_t saltField = ffsCreateJsonField(JSON_KEY_PIN_SALT, FFS_JSON_STRING);
    FfsJsonField_t *rootExpectedFields[] = { &nonceField, &sessionIdField, &canProceedField, &saltField, NULL };

    FFS_CHECK_RESULT(ffsParseJsonObject(startProvisioningSessionResponseValue, rootExpectedFields));

    // Parse the nonce.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&nonceField.value,
            &startProvisioningSessionResponse->nonce));

    // Parse the session ID.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&sessionIdField.value,
            &startProvisioningSessionResponse->sessionId));

    // Parse the 'canProceed' value.
    FFS_CHECK_RESULT(ffsParseJsonBoolean(&canProceedField.value, &startProvisioningSessionResponse->canProceed));

    // Parse the salt.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&saltField.value,
            &startProvisioningSessionResponse->salt));

    return FFS_SUCCESS;
}
