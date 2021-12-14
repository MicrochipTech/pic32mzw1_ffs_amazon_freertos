/** @file ffs_dss_get_wifi_credentials_response.c
 *
 * @brief DSS "get Wi-Fi credentials" response implementation.
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
#include "ffs/dss/model/ffs_dss_get_wifi_credentials_response.h"

#define JSON_KEY_NONCE                      "nonce"
#define JSON_KEY_SESSION_ID                 "sessionId"
#define JSON_KEY_CAN_PROCEED                "canProceed"
#define JSON_KEY_SEQUENCE_NUMBER            "sequenceNumber"
#define JSON_KEY_ALL_CREDENTIALS_RETURNED   "allCredentialsReturned"
#define JSON_KEY_WIFI_CREDENTIALS_LIST      "wifiCredentialsList"

/*
 * Start serializing a DSS "get Wi-Fi credentials" response.
 */
FFS_RESULT ffsDssStartSerializingGetWifiCredentialsResponse(
        FfsDssGetWifiCredentialsResponse_t *getWifiCredentialsResponse,
        FfsStream_t *outputStream)
{
    (void) getWifiCredentialsResponse;
    (void) outputStream;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Add credentials to a DSS "get Wi-Fi credentials" response.
 */
FFS_RESULT ffsDssAddCredentialsToSerializedGetWifiCredentialsResponse(
        FfsDssWifiCredentials_t *credentials, FfsStream_t *outputStream)
{
    (void) credentials;
    (void) outputStream;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Complete serializing a DSS "get Wi-Fi credentials" response.
 */
FFS_RESULT ffsDssFinishSerializingGetWifiCredentialsResponse(FfsStream_t *outputStream)
{
    (void) outputStream;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Deserialize a DSS "get Wi-Fi credentials" response.
 */
FFS_RESULT ffsDssDeserializeGetWifiCredentialsResponse(
        FfsJsonValue_t *getWifiCredentialsResponseValue,
        FfsDssGetWifiCredentialsResponse_t *getWifiCredentialsResponse,
        FfsJsonValue_t *wifiCredentialsListValue)
{
    // Zero out the "get Wi-Fi credentials" response.
    memset(getWifiCredentialsResponse, 0, sizeof(*getWifiCredentialsResponse));

    /* Parse {"nonce":"...",
            "canProceed":"...",
            "sequenceNumber":...,
            "allCredentialsReturned":"...",
            "wifiCredentialsList":[...]} */
    FfsJsonField_t nonceField = ffsCreateJsonField(JSON_KEY_NONCE, FFS_JSON_STRING);
    FfsJsonField_t canProceedField = ffsCreateJsonField(JSON_KEY_CAN_PROCEED, FFS_JSON_BOOLEAN);
    FfsJsonField_t sequenceNumberField = ffsCreateJsonField(JSON_KEY_SEQUENCE_NUMBER, FFS_JSON_NUMBER);
    FfsJsonField_t allCredentialsReturnedField = ffsCreateJsonField(JSON_KEY_ALL_CREDENTIALS_RETURNED, FFS_JSON_BOOLEAN);
    FfsJsonField_t wifiCredentialsListField = ffsCreateJsonField(JSON_KEY_WIFI_CREDENTIALS_LIST, FFS_JSON_ARRAY);

    FfsJsonField_t *getWifiCredentialsResponseExpectedFields[] = {
            &nonceField,
            &canProceedField,
            &sequenceNumberField,
            &allCredentialsReturnedField,
            &wifiCredentialsListField,
            NULL };

    FFS_CHECK_RESULT(ffsParseJsonObject(getWifiCredentialsResponseValue,
            getWifiCredentialsResponseExpectedFields));

    // Parse the 'canProceed' value.
    FFS_CHECK_RESULT(ffsParseJsonBoolean(&canProceedField.value, &getWifiCredentialsResponse->canProceed));

    // Short-circuit if the 'canProceed' value is false.
    if (!getWifiCredentialsResponse->canProceed) {
        FFS_FAIL(FFS_ERROR);
    }

    // Parse the nonce.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&nonceField.value, &getWifiCredentialsResponse->nonce));

    // Parse the 'sequenceNumber' value.
    FFS_CHECK_RESULT(ffsParseJsonUint32(&sequenceNumberField.value, &getWifiCredentialsResponse->sequenceNumber));

    // Parse the 'allCredentialsReturned' value.
    FFS_CHECK_RESULT(ffsParseJsonBoolean(&allCredentialsReturnedField.value, &getWifiCredentialsResponse->allCredentialsReturned));

    // Copy the credentials list JSON value (which may be empty).
    *wifiCredentialsListValue = wifiCredentialsListField.value;

    return FFS_SUCCESS;
}
