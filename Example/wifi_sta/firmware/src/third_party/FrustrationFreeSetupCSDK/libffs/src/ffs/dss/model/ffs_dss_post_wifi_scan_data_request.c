/** @file ffs_dss_get_wifi_credentials_request.c
 *
 * @brief DSS "post Wi-Fi scan data" request implementation.
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
#include "ffs/dss/model/ffs_dss_post_wifi_scan_data_request.h"

#include <stdbool.h>

#define JSON_KEY_NONCE                    "nonce"
#define JSON_KEY_SESSION_ID               "sessionId"
#define JSON_KEY_SEQUENCE_NUMBER          "sequenceNumber"
#define JSON_KEY_WIFI_SCAN_DATA_LIST      "wifiScanDataList"

/*
 * Start serializing a DSS "post Wi-Fi scan data" request.
 */
FFS_RESULT ffsDssStartSerializingPostWifiScanDataRequest(
        FfsDssPostWifiScanDataRequest_t *postWifiScanDataRequest, FfsStream_t *outputStream)
{
    // Start the "post Wi-Fi scan data" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the nonce field.
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_NONCE, postWifiScanDataRequest->nonce,
            outputStream));

    // Serialize the session ID field.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_SESSION_ID, postWifiScanDataRequest->sessionId,
            outputStream));

    // Serialize the device details field.
    FFS_CHECK_RESULT(ffsDssSerializeDeviceDetailsField(&postWifiScanDataRequest->deviceDetails,
            outputStream));

    // Serialize the sequence number.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonUint32Field(JSON_KEY_SEQUENCE_NUMBER, postWifiScanDataRequest->sequenceNumber,
            outputStream));

    // Start the Wi-Fi scan result list.
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonStringKey(JSON_KEY_WIFI_SCAN_DATA_LIST, outputStream));
    FFS_CHECK_RESULT(ffsEncodeJsonArrayStart(outputStream));

    return FFS_SUCCESS;
}

/*
 * Add a scan result to a DSS "post Wi-Fi scan data" request.
 */
FFS_RESULT ffsDssAddScanResultToSerializedPostWifiScanDataRequest(
        FfsDssWifiScanResult_t *scanResult, FfsStream_t *outputStream)
{
    // Work with a copy of the output stream.
    FfsStream_t outputStreamCopy = *outputStream;

    // There should be something already in the buffer.
    if (FFS_STREAM_DATA_SIZE(outputStreamCopy) < 1) {
        FFS_FAIL(FFS_ERROR);
    }

    // Is there already a scan result in the list?
    if (FFS_STREAM_NEXT_WRITE(outputStreamCopy)[-1] != '[') {

        // Add a separator.
        FFS_CHECK_RESULT(ffsEncodeJsonSeparator(&outputStreamCopy));
    }

    // Serialize the scan result.
    FFS_CHECK_RESULT(ffsDssSerializeWifiScanResult(scanResult, &outputStreamCopy));

    // Make sure that there is still enough space for closing out the request.
    if (FFS_STREAM_SPACE_SIZE(outputStreamCopy) < 2) {
        FFS_FAIL(FFS_OVERRUN);
    }

    // Update the original output stream.
    *outputStream = outputStreamCopy;

    return FFS_SUCCESS;
}

/*
 * Complete serializing a DSS "post Wi-Fi scan data" request.
 */
FFS_RESULT ffsDssFinishSerializingPostWifiScanDataRequest(FfsStream_t *outputStream)
{
    // Complete the scan results list.
    FFS_CHECK_RESULT(ffsEncodeJsonArrayEnd(outputStream));

    // Finish the "post Wi-Fi scan data" request object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    return FFS_SUCCESS;
}

/*
 * Deserialize a DSS "post Wi-Fi scan data" request.
 */
FFS_RESULT ffsDssDeserializePostWifiScanDataRequest(
        FfsJsonValue_t *postWifiScanDataRequestValue,
        FfsDssPostWifiScanDataRequest_t *postWifiScanDataRequest,
        FfsJsonValue_t *wifiScanDataValue)
{
    (void) postWifiScanDataRequestValue;
    (void) postWifiScanDataRequest;
    (void) wifiScanDataValue;

    return FFS_NOT_IMPLEMENTED;
}

