/** @file ffs_dss_operation_start_provisioning_session.c
 *
 * @brief "Start provisioning session" operation implementation.
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
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/dss/model/ffs_dss_start_provisioning_session_request.h"
#include "ffs/dss/model/ffs_dss_start_provisioning_session_response.h"
#include "ffs/dss/ffs_dss_client.h"
#include "ffs/dss/ffs_dss_operation.h"
#include "ffs/dss/ffs_dss_operation_compute_configuration_data.h"

// Static function prototypes.
static FFS_RESULT ffsConstructStartProvisioningSessionHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        FfsStream_t *bodyStream);
static FFS_RESULT ffsHandleStartProvisioningSessionHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer);

/** @brief "start provisioning session" operation data.
 */
static const FfsDssOperationData_t FFS_DSS_OPERATION_DATA_START_PROVISIONING_SESSION = {
    .id = FFS_DSS_OPERATION_ID_START_PROVISIONING_SESSION,
    .name = "START PROVISIONING SESSION",
    .path = FFS_DSS_WIFI_PROVISIONEE_API_PATH("startProvisioningSession"),
    .httpCallbacks = {
        .handleStatusCode = ffsDssClientHandleStatusCode,
        .handleHeader = ffsDssClientHandleHeader,
        .handleBody = ffsHandleStartProvisioningSessionHttpResponseBody,
        .handleRedirect = ffsDssClientHandleRedirect,
        .beforeRetry = ffsDssClientBeforeRetry
    }
};

/** @brief DSS "start provisioning session" operation data structure.
 */
typedef struct {
    bool *canProceed;
    FfsStream_t *saltStream;
} FfsStartProvisioningSessionOperationData_t;

/*
 * Execute the "start provisioning session" operation.
 */
FFS_RESULT ffsDssStartProvisioningSession(FfsDssClientContext_t *dssClientContext,
        bool *canProceed, FfsStream_t *saltStream)
{
    // Generate a new nonce.
    FFS_CHECK_RESULT(ffsDssClientRefreshNonce(dssClientContext));

    // Reuse the shared buffer.
    FfsStream_t bodyStream = dssClientContext->bodyStream;

    // Fill in the request body.
    FFS_CHECK_RESULT(ffsConstructStartProvisioningSessionHttpRequestBody(dssClientContext,
            &bodyStream));

    // Create the operation data structure.
    FfsStartProvisioningSessionOperationData_t operationData = {
        .canProceed = canProceed,
        .saltStream = saltStream
    };

    // Send the request and process the response.
    FFS_CHECK_RESULT(ffsDssClientExecute(dssClientContext,
            &FFS_DSS_OPERATION_DATA_START_PROVISIONING_SESSION, &bodyStream, &operationData));

    return FFS_SUCCESS;
}

/** @brief Construct the "start provisioning session" HTTP request body.
 */
static FFS_RESULT ffsConstructStartProvisioningSessionHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        FfsStream_t *bodyStream)
{
    // Construct the request.
    FfsDssStartProvisioningSessionRequest_t startProvisioningSessionRequest = {
        .nonce = (const char *) FFS_STREAM_NEXT_READ(dssClientContext->nonceStream)
    };

    // Serialize it.
    FFS_CHECK_RESULT(ffsDssSerializeStartProvisioningSessionRequest(&startProvisioningSessionRequest,
            bodyStream));

    return FFS_SUCCESS;
}

/** @brief Handle the "start provisioning session" HTTP response body.
 */
static FFS_RESULT ffsHandleStartProvisioningSessionHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer)
{
    FfsDssHttpCallbackData_t *callbackData = (FfsDssHttpCallbackData_t *) callbackDataPointer;
    FfsStartProvisioningSessionOperationData_t *operationData =
            (FfsStartProvisioningSessionOperationData_t *)callbackData->operationCallbackDataPointer;

    // Validate the signature.
    FFS_CHECK_RESULT(ffsDssClientHandleBody(bodyStream, callbackDataPointer));

    // Initialize the root JSON object.
    FfsJsonValue_t rootJsonObject;
    FFS_CHECK_RESULT(ffsInitializeJsonObject(bodyStream, &rootJsonObject));

    // Deserialize the response.
    FfsDssStartProvisioningSessionResponse_t response;
    FFS_CHECK_RESULT(ffsDssDeserializeStartProvisioningSessionResponse(&rootJsonObject,
            &response));

    // Save the session ID.
    FFS_CHECK_RESULT(ffsDssClientSetSessionId(callbackData->dssClientContext,
            response.sessionId));

    // Save the 'canProceed' value.
    *operationData->canProceed = response.canProceed;

    // Save the salt.
    FFS_CHECK_RESULT(ffsWriteStringToStream(response.salt, operationData->saltStream));

    return FFS_SUCCESS;
}
