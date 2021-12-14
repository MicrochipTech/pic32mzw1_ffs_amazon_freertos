/** @file ffs_dss_post_wifi_scan_data_response.c
 *
 * @brief DSS "post Wi-Fi scan data" response implementation.
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
#include "ffs/dss/model/ffs_dss_post_wifi_scan_data_response.h"

#define JSON_KEY_NONCE                      "nonce"
#define JSON_KEY_SESSION_ID                 "sessionId"
#define JSON_KEY_CAN_PROCEED                "canProceed"
#define JSON_KEY_SEQUENCE_NUMBER            "sequenceNumber"
#define JSON_KEY_TOTAL_CREDENTIALS_FOUND    "totalCredentialsFound"
#define JSON_KEY_ALL_CREDENTIALS_FOUND      "allCredentialsFound"

/*
 * Serialize a DSS "post Wi-Fi scan data" response.
 */
FFS_RESULT ffsDssSerializePostWifiScanDataResponse(
        FfsDssPostWifiScanDataResponse_t *postWifiScanDataResponse,
        FfsStream_t *outputStream)
{
    (void) postWifiScanDataResponse;
    (void) outputStream;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Deserialize a DSS "post Wi-Fi scan data" response.
 */
FFS_RESULT ffsDssDeserializePostWifiScanDataResponse(
        FfsJsonValue_t *postWifiScanDataResponseValue,
        FfsDssPostWifiScanDataResponse_t *postWifiScanDataResponse)
{
    // Zero out the "post Wi-Fi scan data" response.
    memset(postWifiScanDataResponse, 0, sizeof(*postWifiScanDataResponse));

    /* Parse { "nonce":"...",
            "sessionId":"...",
            "canProceed":"...",
            "sequenceNumber":...,
            "totalCredentialsFound":...,
            "allCredentialsFound":"..." } */
    FfsJsonField_t nonceField = ffsCreateJsonField(JSON_KEY_NONCE, FFS_JSON_STRING);
    FfsJsonField_t sessionIdField = ffsCreateJsonField(JSON_KEY_SESSION_ID, FFS_JSON_STRING);
    FfsJsonField_t canProceedField = ffsCreateJsonField(JSON_KEY_CAN_PROCEED, FFS_JSON_BOOLEAN);
    FfsJsonField_t sequenceNumberField = ffsCreateJsonField(JSON_KEY_SEQUENCE_NUMBER, FFS_JSON_NUMBER);
    FfsJsonField_t totalCredentialsFoundField = ffsCreateJsonField(JSON_KEY_TOTAL_CREDENTIALS_FOUND, FFS_JSON_NUMBER);
    FfsJsonField_t allCredentialsFoundField = ffsCreateJsonField(JSON_KEY_ALL_CREDENTIALS_FOUND, FFS_JSON_BOOLEAN);

    FfsJsonField_t *postWifiScanDataResponseExpectedFields[] = {
            &nonceField,
            &sessionIdField,
            &canProceedField,
            &sequenceNumberField,
            &totalCredentialsFoundField,
            &allCredentialsFoundField,
            NULL };

    FFS_CHECK_RESULT(ffsParseJsonObject(postWifiScanDataResponseValue,
            postWifiScanDataResponseExpectedFields));

    // Parse the 'canProceed' value.
    FFS_CHECK_RESULT(ffsParseJsonBoolean(&canProceedField.value, &postWifiScanDataResponse->canProceed));

    // Short-circuit if the 'canProceed' value is false.
    if (!postWifiScanDataResponse->canProceed) {
        FFS_FAIL(FFS_ERROR);
    }

    // Parse the nonce.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&nonceField.value, &postWifiScanDataResponse->nonce));

    // Parse the session ID.
    FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8String(&sessionIdField.value, &postWifiScanDataResponse->sessionId));

    // Parse the 'sequenceNumber' value.
    FFS_CHECK_RESULT(ffsParseJsonUint32(&sequenceNumberField.value, &postWifiScanDataResponse->sequenceNumber));

    // Parse the 'totalCredentialsFound' value.
    FFS_CHECK_RESULT(ffsParseJsonUint32(&totalCredentialsFoundField.value, &postWifiScanDataResponse->totalCredentialsFound));

    // Parse the 'allCredentialsFound' value.
    FFS_CHECK_RESULT(ffsParseJsonBoolean(&allCredentialsFoundField.value, &postWifiScanDataResponse->allCredentialsFound));

    return FFS_SUCCESS;
}

