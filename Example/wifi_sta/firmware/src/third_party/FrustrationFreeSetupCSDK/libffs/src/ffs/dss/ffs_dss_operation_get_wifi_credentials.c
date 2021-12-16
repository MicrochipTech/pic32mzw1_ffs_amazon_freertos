/** @file ffs_dss_operation_get_wifi_credentials.c
 *
 * @brief "Get Wi-Fi credentials" operation.
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
#include "ffs/common/ffs_http.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"
#include "ffs/conversion/ffs_convert_device_details.h"
#include "ffs/dss/model/ffs_dss_get_wifi_credentials_request.h"
#include "ffs/dss/model/ffs_dss_get_wifi_credentials_response.h"
#include "ffs/dss/model/ffs_dss_wifi_credentials.h"
#include "ffs/dss/ffs_dss_client.h"
#include "ffs/dss/ffs_dss_operation.h"
#include "ffs/dss/ffs_dss_operation_get_wifi_credentials.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Static function prototypes.
static FFS_RESULT ffsConstructGetWifiCredentialsHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        uint32_t sequenceNumber, FfsStream_t *bodyStream);
static FFS_RESULT ffsHandleGetWifiCredentialsHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer);

/** @brief "Get Wi-Fi credentials" operation data.
 */
static const FfsDssOperationData_t FFS_DSS_OPERATION_DATA_GET_WIFI_CREDENTIALS = {
    .id = FFS_DSS_OPERATION_ID_GET_WIFI_CREDENTIALS,
    .name = "GET WIFI CREDENTIALS",
    .path = FFS_DSS_WIFI_PROVISIONEE_API_PATH("getWifiCredentials"),
    .httpCallbacks = {
        .handleStatusCode = ffsDssClientHandleStatusCode,
        .handleHeader = ffsDssClientHandleHeader,
        .handleBody = ffsHandleGetWifiCredentialsHttpResponseBody,
        .handleRedirect = ffsDssClientHandleRedirect,
        .beforeRetry = ffsDssClientBeforeRetry
    }
};

/** @brief Data structure for response body callback.
 */
typedef struct {
    bool *canProceed;
    FfsDssSaveWifiCredentialsCallback_t saveCredentialsCallback;
    bool *allCredentialsReturned;
} FfsDssGetWifiCredentialsOperationData_t;

/*
 * Execute the "get Wi-Fi credentials" operation.
 */
FFS_RESULT ffsDssGetWifiCredentials(FfsDssClientContext_t *dssClientContext,
        bool *canProceed,
        uint32_t sequenceNumber,
        FfsDssSaveWifiCredentialsCallback_t saveCredentialsCallback,
        bool *allCredentialsReturned)
{
    // Generate a new nonce.
    FFS_CHECK_RESULT(ffsDssClientRefreshNonce(dssClientContext));

    // Reuse the shared buffer.
    FfsStream_t bodyStream = dssClientContext->bodyStream;

    // Fill in the request body.
    FFS_CHECK_RESULT(ffsConstructGetWifiCredentialsHttpRequestBody(dssClientContext,
            sequenceNumber, &bodyStream));

    // Create the operation data structure.
    FfsDssGetWifiCredentialsOperationData_t operationData = {
        .canProceed = canProceed,
        .saveCredentialsCallback = saveCredentialsCallback,
        .allCredentialsReturned = allCredentialsReturned
    };

    // Send the request and process the response.
    FFS_CHECK_RESULT(ffsDssClientExecute(dssClientContext,
            &FFS_DSS_OPERATION_DATA_GET_WIFI_CREDENTIALS, &bodyStream, &operationData));

    return FFS_SUCCESS;
}

/** @brief Construct the "get Wi-Fi credentials" HTTP request body.
 */
static FFS_RESULT ffsConstructGetWifiCredentialsHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        uint32_t sequenceNumber, FfsStream_t *bodyStream)
{
    // Start the request.
    FfsDssGetWifiCredentialsRequest_t getWifiCredentialsRequest = {
        .sequenceNumber = sequenceNumber
    };
    FFS_CHECK_RESULT(ffsDssClientGetSessionId(dssClientContext, &getWifiCredentialsRequest.sessionId));
    FFS_CHECK_RESULT(ffsDssClientGetNonce(dssClientContext, &getWifiCredentialsRequest.nonce));
    FFS_CHECK_RESULT(ffsConstructDssDeviceDetails(dssClientContext->userContext, bodyStream,
            &getWifiCredentialsRequest.deviceDetails));
    FFS_CHECK_RESULT(ffsDssSerializeGetWifiCredentialsRequest(&getWifiCredentialsRequest,
            bodyStream));

    return FFS_SUCCESS;
}

/** @brief Handle the "get Wi-Fi credentials" HTTP response body.
 */
static FFS_RESULT ffsHandleGetWifiCredentialsHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer)
{
    FfsDssHttpCallbackData_t *callbackData = (FfsDssHttpCallbackData_t *) callbackDataPointer;
    FfsDssClientContext_t *dssClientContext = callbackData->dssClientContext;
    FfsDssGetWifiCredentialsOperationData_t *operationData =
            (FfsDssGetWifiCredentialsOperationData_t *)callbackData->operationCallbackDataPointer;

    // Get the "save Wi-Fi credentials" callback.
    FfsDssSaveWifiCredentialsCallback_t saveCredentialsCallback = operationData->saveCredentialsCallback;

    // Validate the signature.
    FFS_CHECK_RESULT(ffsDssClientHandleBody(bodyStream, callbackDataPointer));

    // Initialize the root JSON object.
    FfsJsonValue_t rootJsonObject;
    FFS_CHECK_RESULT(ffsInitializeJsonObject(bodyStream, &rootJsonObject));

    // Deserialize the response.
    FfsDssGetWifiCredentialsResponse_t getWifiCredentialsResponse;
    FfsJsonValue_t wifiCredentialsListValue;
    FFS_CHECK_RESULT(ffsDssDeserializeGetWifiCredentialsResponse(&rootJsonObject,
            &getWifiCredentialsResponse, &wifiCredentialsListValue));

    // Save the 'canProceed' value.
    *operationData->canProceed = getWifiCredentialsResponse.canProceed;

    // Save the 'allCredentialsReturned' value.
    *operationData->allCredentialsReturned = getWifiCredentialsResponse.allCredentialsReturned;

    // Iterate through the credentials list.
    for (;;) {

        FfsJsonValue_t wifiCredentialsJsonObject;

        // Get the next value from the array.
        bool isDone = false;
        FFS_CHECK_RESULT(ffsParseJsonValue(&wifiCredentialsListValue, &wifiCredentialsJsonObject, &isDone));
        if (isDone) {
            break;
        }

        // Parse the Wi-Fi credentials.
        FfsDssWifiCredentials_t wifiCredentials;
        FFS_RESULT result = ffsDssDeserializeWifiCredentials(&wifiCredentialsJsonObject,
                &wifiCredentials);
        if (result != FFS_SUCCESS) {

            // Ignore parsing errors - use only valid networks.
            ffsLogWarning("Error parsing a Wi-Fi configuration from the response. Ignoring the entry.");
            continue;
        }

        // Save the credentials?
        if (saveCredentialsCallback) {
            FFS_CHECK_RESULT(saveCredentialsCallback(dssClientContext->userContext,
                    &wifiCredentials));
        }
    }

    return FFS_SUCCESS;
}
