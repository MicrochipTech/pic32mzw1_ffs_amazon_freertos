/** @file ffs_dss_get_wifi_credentials_request.c
 *
 * @brief DSS "get Wi-Fi credentials" request implementation.
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
#include "ffs/dss/model/ffs_dss_device_details.h"
#include "ffs/dss/model/ffs_dss_get_wifi_credentials_request.h"

#include <stdbool.h>

#define JSON_KEY_NONCE                  "nonce"
#define JSON_KEY_SESSION_ID             "sessionId"
#define JSON_KEY_SEQUENCE_NUMBER        "sequenceNumber"
#define JSON_KEY_WIFI_SCAN_DATA_LIST    "wifiScanDataList"

/*
 * Start serializing a DSS "get Wi-Fi credentials" request.
 */
FFS_RESULT ffsDssSerializeGetWifiCredentialsRequest(
        FfsDssGetWifiCredentialsRequest_t *getWifiCredentialsRequest, FfsStream_t *outputStream)
{
    // Start the "get Wi-Fi credentials" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the nonce field.
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_NONCE, getWifiCredentialsRequest->nonce,
            outputStream));

    // Serialize the session ID field.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_SESSION_ID, getWifiCredentialsRequest->sessionId,
            outputStream));

    // Serialize the device details field.
    FFS_CHECK_RESULT(ffsDssSerializeDeviceDetailsField(&getWifiCredentialsRequest->deviceDetails,
            outputStream));

    // Serialize the sequence number.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonUint32Field(JSON_KEY_SEQUENCE_NUMBER, getWifiCredentialsRequest->sequenceNumber,
            outputStream));

    // Finish the "get Wi-Fi credentials" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    return FFS_SUCCESS;
}

/*
 * Deserialize a DSS "get Wi-Fi credentials" request.
 */
FFS_RESULT ffsDssDeserializeGetWifiCredentialsRequest(
        FfsJsonValue_t *getWifiCredentialsRequestValue,
        FfsDssGetWifiCredentialsRequest_t *getWifiCredentialsRequest,
        FfsJsonValue_t *wifiCredentialsListValue)
{
    (void) getWifiCredentialsRequestValue;
    (void) getWifiCredentialsRequest;
    (void) wifiCredentialsListValue;

    return FFS_NOT_IMPLEMENTED;
}
