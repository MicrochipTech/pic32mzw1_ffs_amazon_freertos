/** @file ffs_dss_operation_report.c
 *
 * @brief "Report" operation implementation.
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
#include "ffs/common/ffs_error_details.h"
#include "ffs/common/ffs_json.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"
#include "ffs/conversion/ffs_convert_device_details.h"
#include "ffs/dss/model/ffs_dss_registration_details.h"
#include "ffs/dss/model/ffs_dss_report_request.h"
#include "ffs/dss/model/ffs_dss_report_response.h"
#include "ffs/dss/ffs_dss_client.h"
#include "ffs/dss/ffs_dss_operation.h"
#include "ffs/dss/ffs_dss_operation_report.h"

#include <string.h>

// Static function prototypes.
static FFS_RESULT ffsConstructReportHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        FFS_DSS_WIFI_PROVISIONEE_STATE provisioneeState, FFS_DSS_REPORT_RESULT stateTransitionResult,
        FFS_DSS_REGISTRATION_STATE registrationState,
        FfsDssGetConnectionAttemptsCallback_t getConnectionAttempts,
        FfsDssWifiConnectionAttempt_t *wifiConnectionAttempt,
        FfsStream_t *bodyStream);
static FFS_RESULT ffsHandleReportHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer);

/** @brief "Report" operation data.
 */
static const FfsDssOperationData_t FFS_DSS_OPERATION_DATA_REPORT = {
    .id = FFS_DSS_OPERATION_ID_REPORT,
    .name = "REPORT",
    .path = FFS_DSS_WIFI_PROVISIONEE_API_PATH("report"),
    .httpCallbacks = {
        .handleStatusCode = ffsDssClientHandleStatusCode,
        .handleHeader = ffsDssClientHandleHeader,
        .handleBody = ffsHandleReportHttpResponseBody,
        .handleRedirect = ffsDssClientHandleRedirect,
        .beforeRetry = ffsDssClientBeforeRetry
    }
};

/** @brief Report operation data structure.
 */
typedef struct {
    bool *canProceed;
    FFS_DSS_WIFI_PROVISIONEE_STATE *nextProvisioneeState;
} FfsDssReportOperationData_t;

/*
 * Execute the "report" operation.
 */
FFS_RESULT ffsDssReport(FfsDssClientContext_t *dssClientContext,
        FFS_DSS_WIFI_PROVISIONEE_STATE provisioneeState,
        FFS_DSS_REPORT_RESULT stateTransitionResult,
        FFS_DSS_REGISTRATION_STATE registrationState,
        bool *canProceed,
        FFS_DSS_WIFI_PROVISIONEE_STATE *nextProvisioneeState,
        FfsDssGetConnectionAttemptsCallback_t getConnectionAttemptsCallback,
        FfsDssWifiConnectionAttempt_t *wifiConnectionAttempt)
{
    // Increment the sequence number.
    dssClientContext->sequenceNumber++;

    // Generate a new nonce.
    FFS_CHECK_RESULT(ffsDssClientRefreshNonce(dssClientContext));

    // Reuse the shared buffer.
    FfsStream_t bodyStream = dssClientContext->bodyStream;

    // Fill in the request body.
    FFS_CHECK_RESULT(ffsConstructReportHttpRequestBody(dssClientContext, provisioneeState,
            stateTransitionResult, registrationState, getConnectionAttemptsCallback, wifiConnectionAttempt, &bodyStream));

    // Create the operation data structure.
    FfsDssReportOperationData_t operationData = {
        .canProceed = canProceed,
        .nextProvisioneeState = nextProvisioneeState
    };

    // Send the request and process the response.
    FFS_CHECK_RESULT(ffsDssClientExecute(dssClientContext,
            &FFS_DSS_OPERATION_DATA_REPORT, &bodyStream, &operationData));

    return FFS_SUCCESS;
}

/*
 * Add a connection attempt to a report.
 */
FFS_RESULT ffsDssReportAddConnectionAttempt(void *callbackDataPointer,
        FfsDssWifiConnectionDetails_t *connectionAttempt)
{
    FFS_CHECK_RESULT(ffsDssAddConnectionAttemptToSerializedReportRequest(connectionAttempt,
            (FfsStream_t *) callbackDataPointer));

    return FFS_SUCCESS;
}

/** @brief Construct the "report" HTTP request body.
 */
static FFS_RESULT ffsConstructReportHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        FFS_DSS_WIFI_PROVISIONEE_STATE provisioneeState, FFS_DSS_REPORT_RESULT stateTransitionResult,
        FFS_DSS_REGISTRATION_STATE registrationState,
        FfsDssGetConnectionAttemptsCallback_t getConnectionAttemptsCallback,
        FfsDssWifiConnectionAttempt_t *wifiConnectionAttempt,
        FfsStream_t *bodyStream)
{
    // Construct the "report" request object (excluding connection attempts).
    FfsDssReportRequest_t reportRequest = {
        .sequenceNumber = dssClientContext->sequenceNumber,
        .provisioneeState = provisioneeState,
        .registrationState = registrationState,
        .stateTransitionResult = stateTransitionResult
    };
    FFS_CHECK_RESULT(ffsDssClientGetNonce(dssClientContext, &reportRequest.nonce));
    FFS_CHECK_RESULT(ffsDssClientGetSessionId(dssClientContext, &reportRequest.sessionId));
    FFS_CHECK_RESULT(ffsConstructDssDeviceDetails(dssClientContext->userContext, bodyStream,
            &reportRequest.deviceDetails));

    // Serialize it.
    FFS_CHECK_RESULT(ffsDssStartSerializingReportRequest(&reportRequest, bodyStream));

    // Are there Wi-Fi connection attempts to report?
    if (getConnectionAttemptsCallback) {
        FFS_CHECK_RESULT(getConnectionAttemptsCallback(dssClientContext->userContext, wifiConnectionAttempt, bodyStream));
    }

    // End the request.
    FFS_CHECK_RESULT(ffsDssFinishSerializingReportRequest(bodyStream));

    return FFS_SUCCESS;
}

/** @brief Handle the "report" HTTP response body.
 */
static FFS_RESULT ffsHandleReportHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer)
{

    FfsDssHttpCallbackData_t *callbackData = (FfsDssHttpCallbackData_t *) callbackDataPointer;
    FfsDssReportOperationData_t *operationData = (FfsDssReportOperationData_t *)callbackData->operationCallbackDataPointer;

    // Validate the signature.
    FFS_CHECK_RESULT(ffsDssClientHandleBody(bodyStream, callbackDataPointer));

    // Initialize the root JSON object.
    FfsJsonValue_t rootJsonObject;
    FFS_CHECK_RESULT(ffsInitializeJsonObject(bodyStream, &rootJsonObject));

    // Parse the response.
    FfsDssReportResponse_t reportResponse;
    FFS_CHECK_RESULT(ffsDssDeserializeReportResponse(&rootJsonObject, &reportResponse));

    // Save the 'canProceed' value.
    *operationData->canProceed = reportResponse.canProceed;

    // Set the next Wi-Fi provisioning state.
    *operationData->nextProvisioneeState = reportResponse.nextProvisioningState;

    return FFS_SUCCESS;
}
