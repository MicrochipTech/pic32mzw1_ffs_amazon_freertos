/** @file ffs_dss_operation_post_wifi_scan_data.c
 *
 * @brief "Post Wi-Fi scan data" operation.
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
#include "ffs/common/ffs_configuration_map.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/conversion/ffs_convert_device_details.h"
#include "ffs/dss/model/ffs_dss_post_wifi_scan_data_request.h"
#include "ffs/dss/model/ffs_dss_post_wifi_scan_data_response.h"
#include "ffs/dss/ffs_dss_operation_post_wifi_scan_data.h"

// Static function prototypes.
static FFS_RESULT ffsConstructPostWifiScanDataHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        uint32_t sequenceNumber,
        FfsDssGetWifiScanResultsCallback_t getScanResultsCallback,
        FfsDssWifiScanResult_t *wifiScanResult,
        FfsStream_t *bodyStream);
static FFS_RESULT ffsHandlePostWifiScanDataHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer);

/** @brief "Post Wi-Fi scan data" operation data.
 */
static const FfsDssOperationData_t FFS_DSS_OPERATION_DATA_POST_WIFI_SCAN_DATA = {
    .id = FFS_DSS_OPERATION_ID_POST_WIFI_SCAN_DATA,
    .name = "POST WIFI SCAN DATA",
    .path = FFS_DSS_WIFI_PROVISIONEE_API_PATH("postWifiScanData"),
    .httpCallbacks = {
        .handleStatusCode = ffsDssClientHandleStatusCode,
        .handleHeader = ffsDssClientHandleHeader,
        .handleBody = ffsHandlePostWifiScanDataHttpResponseBody,
        .handleRedirect = ffsDssClientHandleRedirect,
        .beforeRetry = ffsDssClientBeforeRetry
    }
};

/** @brief Data structure for response body callback.
 */
typedef struct {
    bool *canProceed;
    uint32_t *totalCredentialsFound;
    bool *allCredentialsFound;
} FfsDssPostWifiScanDataOperationData_t;

/*
 * Execute the "post Wi-Fi scan data" operation.
 */
FFS_RESULT ffsDssPostWifiScanData(FfsDssClientContext_t *dssClientContext,
        bool *canProceed,
        uint32_t sequenceNumber,
        FfsDssGetWifiScanResultsCallback_t getScanResultsCallback,
        FfsDssWifiScanResult_t *wifiScanResult,
        uint32_t *totalCredentialsFound,
        bool *allCredentialsFound)
{
    // Generate a new nonce.
    FFS_CHECK_RESULT(ffsDssClientRefreshNonce(dssClientContext));

    // Reuse the shared buffer.
    FfsStream_t bodyStream = dssClientContext->bodyStream;

    // Fill in the request body.
    FFS_CHECK_RESULT(ffsConstructPostWifiScanDataHttpRequestBody(dssClientContext,
            sequenceNumber, getScanResultsCallback, wifiScanResult, &bodyStream));

    // Create the operation data structure.
    FfsDssPostWifiScanDataOperationData_t operationData = {
        .canProceed = canProceed,
        .totalCredentialsFound = totalCredentialsFound,
        .allCredentialsFound = allCredentialsFound
    };

    // Send the request and process the response.
    FFS_CHECK_RESULT(ffsDssClientExecute(dssClientContext,
            &FFS_DSS_OPERATION_DATA_POST_WIFI_SCAN_DATA, &bodyStream, &operationData));

    return FFS_SUCCESS;
}

/*
 * Add a Wi-Fi scan result to a "post Wi-Fi scan data" call.
 */
FFS_RESULT ffsDssPostWifiScanDataAddScanResult(void *callbackDataPointer,
        FfsDssWifiScanResult_t *scanResult)
{
    // DSS can only handle WPA/PSK, WEP and open networks.
    if (scanResult->securityProtocol == FFS_DSS_WIFI_SECURITY_PROTOCOL_WPA_PSK
            || scanResult->securityProtocol == FFS_DSS_WIFI_SECURITY_PROTOCOL_WEP
            || scanResult->securityProtocol == FFS_DSS_WIFI_SECURITY_PROTOCOL_OPEN) {

        // Add the network.
        FFS_CHECK_RESULT(ffsDssAddScanResultToSerializedPostWifiScanDataRequest(scanResult,
                (FfsStream_t *) callbackDataPointer));
    }

    return FFS_SUCCESS;
}

/** @brief Construct the "post Wi-Fi scan data" HTTP request body.
 */
static FFS_RESULT ffsConstructPostWifiScanDataHttpRequestBody(
        FfsDssClientContext_t *dssClientContext,
        uint32_t sequenceNumber,
        FfsDssGetWifiScanResultsCallback_t getScanResultsCallback,
        FfsDssWifiScanResult_t *wifiScanResult,
        FfsStream_t *bodyStream)
{
    // Start the request.
    FfsDssPostWifiScanDataRequest_t postWifiScanDataRequest = {
        .sequenceNumber = sequenceNumber
    };
    FFS_CHECK_RESULT(ffsDssClientGetSessionId(dssClientContext, &postWifiScanDataRequest.sessionId));
    FFS_CHECK_RESULT(ffsDssClientGetNonce(dssClientContext, &postWifiScanDataRequest.nonce));
    FFS_CHECK_RESULT(ffsConstructDssDeviceDetails(dssClientContext->userContext, bodyStream,
            &postWifiScanDataRequest.deviceDetails));

    // Start serializing the request.
    FFS_CHECK_RESULT(ffsDssStartSerializingPostWifiScanDataRequest(&postWifiScanDataRequest,
            bodyStream));

    // Are there Wi-Fi scan results to report?
    if (getScanResultsCallback) {
        FFS_CHECK_RESULT(getScanResultsCallback(dssClientContext->userContext, wifiScanResult, bodyStream));
    }

    // End the request.
    FFS_CHECK_RESULT(ffsDssFinishSerializingPostWifiScanDataRequest(bodyStream));

    return FFS_SUCCESS;
}

static FFS_RESULT ffsHandlePostWifiScanDataHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer)
{
    FfsDssHttpCallbackData_t *callbackData = (FfsDssHttpCallbackData_t *) callbackDataPointer;
    FfsDssPostWifiScanDataOperationData_t *operationData =
            (FfsDssPostWifiScanDataOperationData_t *)callbackData->operationCallbackDataPointer;

    // Validate the signature.
    FFS_CHECK_RESULT(ffsDssClientHandleBody(bodyStream, callbackDataPointer));

    // Initialize the root JSON object.
    FfsJsonValue_t rootJsonObject;
    FFS_CHECK_RESULT(ffsInitializeJsonObject(bodyStream, &rootJsonObject));

    // Deserialize the response.
    FfsDssPostWifiScanDataResponse_t postWifiScanDataResponse;
    FFS_CHECK_RESULT(ffsDssDeserializePostWifiScanDataResponse(&rootJsonObject,
            &postWifiScanDataResponse));

    // Save the 'canProceed' value.
    *operationData->canProceed = postWifiScanDataResponse.canProceed;

    // Save the 'totalCredentialsFound' value.;
    *operationData->totalCredentialsFound = postWifiScanDataResponse.totalCredentialsFound;

    // Save the 'allCredentialsFound' value.
    *operationData->allCredentialsFound = postWifiScanDataResponse.allCredentialsFound;

    return FFS_SUCCESS;
}
