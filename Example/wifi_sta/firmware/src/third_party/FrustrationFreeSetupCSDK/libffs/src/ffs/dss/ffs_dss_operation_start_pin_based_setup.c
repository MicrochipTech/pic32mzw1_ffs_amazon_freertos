/** @file ffs_dss_operation_start_provisioning_session.c
 *
 * @brief "Start provisioning session" operation.
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

#include "ffs/common/ffs_base64.h"
#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_configuration_map.h"
#include "ffs/common/ffs_json.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/conversion/ffs_convert_device_details.h"
#include "ffs/dss/model/ffs_dss_start_pin_based_setup_request.h"
#include "ffs/dss/model/ffs_dss_start_pin_based_setup_response.h"
#include "ffs/dss/ffs_dss_operation_start_pin_based_setup.h"

// Static function prototypes.
static FFS_RESULT ffsConstructStartPinBasedSetupHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        FfsStream_t *bodyStream, FfsStream_t saltStream);
static FFS_RESULT ffsGetHashedPin(FfsDssClientContext_t *dssClientContext,
        FfsStream_t *bodyStream, FfsStream_t saltStream, const char **hashedPin);
static FFS_RESULT ffsHandleStartPinBasedSetupHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer);

/** @brief "start PIN-based setup" operation data.
 */
static const FfsDssOperationData_t FFS_DSS_OPERATION_DATA_START_PIN_BASED_SETUP = {
    .id = FFS_DSS_OPERATION_ID_START_PIN_BASED_SETUP,
    .name = "START PIN BASED SETUP",
    .path = FFS_DSS_WIFI_PROVISIONEE_API_PATH("startPinBasedSetup"),
    .httpCallbacks = {
        .handleStatusCode = ffsDssClientHandleStatusCode,
        .handleHeader = ffsDssClientHandleHeader,
        .handleBody = ffsHandleStartPinBasedSetupHttpResponseBody,
        .handleRedirect = ffsDssClientHandleRedirect,
        .beforeRetry = ffsDssClientBeforeRetry
    }
};

/*
 * Execute the "start PIN-based setup" operation.
 */
FFS_RESULT ffsDssStartPinBasedSetup(FfsDssClientContext_t *dssClientContext,
        bool *canProceed, FfsStream_t saltStream)
{
    // Generate a new nonce.
    FFS_CHECK_RESULT(ffsDssClientRefreshNonce(dssClientContext));

    // Reuse the shared buffer.
    FfsStream_t bodyStream = dssClientContext->bodyStream;

    // Fill in the request body.
    FFS_CHECK_RESULT(ffsConstructStartPinBasedSetupHttpRequestBody(dssClientContext,
            &bodyStream, saltStream));

    // Send the request and process the response.
    FFS_CHECK_RESULT(ffsDssClientExecute(dssClientContext,
            &FFS_DSS_OPERATION_DATA_START_PIN_BASED_SETUP, &bodyStream, canProceed));

    return FFS_SUCCESS;
}

/*
 * Construct the "start PIN-based setup" HTTP request body.
 */
static FFS_RESULT ffsConstructStartPinBasedSetupHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        FfsStream_t *bodyStream, FfsStream_t saltStream)
{
    // Construct the request.
    FfsDssStartPinBasedSetupRequest_t request = {
        .nonce = NULL
    };
    FFS_CHECK_RESULT(ffsDssClientGetNonce(dssClientContext, &request.nonce));
    FFS_CHECK_RESULT(ffsDssClientGetSessionId(dssClientContext, &request.sessionId));
    FFS_CHECK_RESULT(ffsConstructDssDeviceDetails(dssClientContext->userContext, bodyStream,
            &request.deviceDetails));

    // Serialize the request without the hashed PIN.
    FFS_CHECK_RESULT(ffsDssSerializeStartPinBasedSetupRequest(&request, bodyStream));

    // Get the hashed PIN.
    const char *hashedPin;
    FFS_CHECK_RESULT(ffsGetHashedPin(dssClientContext, bodyStream, saltStream, &hashedPin));

    // Seraliaze it.
    FFS_CHECK_RESULT(ffsDssAddHashedPinToStartPinBasedSetupRequest(hashedPin, bodyStream));

    // Finish the request.
    FFS_CHECK_RESULT(ffsDssFinishSerializingStartPinBasedSetupRequest(bodyStream));

    return FFS_SUCCESS;
}

/*
 * Hash and encode the PIN.
 */
static FFS_RESULT ffsGetHashedPin(FfsDssClientContext_t *dssClientContext,
        FfsStream_t *bodyStream, FfsStream_t saltStream, const char **hashedPin)
{
    struct FfsUserContext_s *userContext = dssClientContext->userContext;

    // Get the PIN.
    FfsMapValue_t pinValue = {
        .type = FFS_MAP_VALUE_TYPE_STRING,
        .stringStream = ffsReuseOutputStreamAsOutput(bodyStream) //!< Write the PIN to the shared buffer.
    };
    FFS_CHECK_RESULT(ffsGetConfigurationValue(userContext, FFS_CONFIGURATION_ENTRY_KEY_PIN, &pinValue));
    FfsStream_t saltedPinStream = pinValue.stringStream;

    // Add the salt.
    FFS_CHECK_RESULT(ffsAppendStream(&saltStream, &saltedPinStream));

    // Hash the salted PIN in place.
    FfsStream_t hashedPinStream = ffsReuseOutputStreamAsOutput(&saltedPinStream);
    FFS_CHECK_RESULT(ffsSha256(userContext, &saltedPinStream, &hashedPinStream));

    // Convert the hashed PIN to base64.
    FfsStream_t base64EncodedPinStream = ffsReuseOutputStreamAsOutput(&hashedPinStream);
    FFS_CHECK_RESULT(ffsEncodeBase64(&hashedPinStream, 0, NULL, &base64EncodedPinStream));

    // Write a NULL terminator.
    FFS_CHECK_RESULT(ffsWriteByteToStream('\0', &base64EncodedPinStream));

    // Move the string to the end of the body buffer.
    FFS_CHECK_RESULT(ffsMoveStreamDataToEnd(&base64EncodedPinStream));

    // Return a pointer to the hashed PIN.
    *hashedPin = (const char *)FFS_STREAM_NEXT_READ(base64EncodedPinStream);

    return FFS_SUCCESS;
}

/*
 * Handle the "start PIN-based setup" HTTP response body.
 */
static FFS_RESULT ffsHandleStartPinBasedSetupHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer) {
    FfsDssHttpCallbackData_t *callbackData = (FfsDssHttpCallbackData_t *) callbackDataPointer;
    bool *canProceed = (bool *)callbackData->operationCallbackDataPointer;

    // Validate the signature.
    FFS_CHECK_RESULT(ffsDssClientHandleBody(bodyStream, callbackDataPointer));

    // Initialize the root JSON object.
    FfsJsonValue_t rootJsonObject;
    FFS_CHECK_RESULT(ffsInitializeJsonObject(bodyStream, &rootJsonObject));

    // Deserialize the response.
    FfsDssStartPinBasedSetupResponse_t response;
    FFS_CHECK_RESULT(ffsDssDeserializeStartPinBasedSetupResponse(&rootJsonObject,
            &response));

    // Save the 'canProceed' value.
    *canProceed = response.canProceed;

    return FFS_SUCCESS;
}
