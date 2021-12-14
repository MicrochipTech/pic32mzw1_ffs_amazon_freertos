/** @file ffs_dss_operation_compute_configuration_data.c
 *
 * @brief "Compute configuration data" operation implementation.
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
#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_registration.h"
#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/conversion/ffs_convert_device_details.h"
#include "ffs/conversion/ffs_convert_json_value.h"
#include "ffs/dss/model/ffs_dss_compute_configuration_data_request.h"
#include "ffs/dss/model/ffs_dss_compute_configuration_data_response.h"
#include "ffs/dss/ffs_dss_client.h"
#include "ffs/dss/ffs_dss_operation.h"
#include "ffs/dss/ffs_dss_operation_compute_configuration_data.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Static function prototypes.
static FFS_RESULT ffsConstructComputeConfigurationDataHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        FfsStream_t *bodyStream);
static FFS_RESULT ffsHandleComputeConfigurationDataHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer);

/** @brief "Compute configuration data" operation data.
 */
static const FfsDssOperationData_t FFS_DSS_OPERATION_DATA_COMPUTE_CONFIGURATION_DATA = {
    .id = FFS_DSS_OPERATION_ID_COMPUTE_CONFIGURATION_DATA,
    .name = "COMPUTE CONFIGURATION DATA",
    .path = FFS_DSS_WIFI_PROVISIONEE_API_PATH("computeConfigurationData"),
    .httpCallbacks = {
        .handleStatusCode = ffsDssClientHandleStatusCode,
        .handleHeader = ffsDssClientHandleHeader,
        .handleBody = ffsHandleComputeConfigurationDataHttpResponseBody,
        .handleRedirect = ffsDssClientHandleRedirect,
        .beforeRetry = ffsDssClientBeforeRetry
    }
};

/** @brief "Save response data" callbacks object.
 */
typedef struct {
    FfsDssSaveRegistrationDetailsCallback_t saveRegistrationDetailsCallback;
    FfsDssSaveConfigurationCallback_t saveConfigurationCallback;
} FfsDssComputeConfigurationDataReponseCallbacks_t;

/*
 * Execute a "compute configuration data" operation.
 */
FFS_RESULT ffsDssComputeConfigurationData(FfsDssClientContext_t *dssClientContext,
        FfsDssSaveRegistrationDetailsCallback_t saveRegistrationDetailsCallback,
        FfsDssSaveConfigurationCallback_t saveConfigurationCallback)
{
    // Generate a new nonce.
    FFS_CHECK_RESULT(ffsDssClientRefreshNonce(dssClientContext));

    // Reuse the shared buffer.
    FfsStream_t bodyStream = dssClientContext->bodyStream;

    // Fill in the request body.
    FFS_CHECK_RESULT(ffsConstructComputeConfigurationDataHttpRequestBody(dssClientContext,
            &bodyStream));

    // Send the request and process the response.
    FfsDssComputeConfigurationDataReponseCallbacks_t responseCallbacks = {
        .saveRegistrationDetailsCallback = saveRegistrationDetailsCallback,
        .saveConfigurationCallback = saveConfigurationCallback
    };
    FFS_CHECK_RESULT(ffsDssClientExecute(dssClientContext,
            &FFS_DSS_OPERATION_DATA_COMPUTE_CONFIGURATION_DATA, &bodyStream, &responseCallbacks));

    return FFS_SUCCESS;
}

/** @brief Construct the "compute configuration data" HTTP request body.
 */
static FFS_RESULT ffsConstructComputeConfigurationDataHttpRequestBody(FfsDssClientContext_t *dssClientContext,
        FfsStream_t *bodyStream)
{
    // Construct the request.
    FfsDssComputeConfigurationDataRequest_t computeConfigurationDataRequest = {
        .nonce = NULL
    };
    FFS_CHECK_RESULT(ffsDssClientGetSessionId(dssClientContext,
            &computeConfigurationDataRequest.sessionId));
    FFS_CHECK_RESULT(ffsDssClientGetNonce(dssClientContext, &computeConfigurationDataRequest.nonce));
    FFS_CHECK_RESULT(ffsConstructDssDeviceDetails(dssClientContext->userContext, bodyStream,
            &computeConfigurationDataRequest.deviceDetails));

    // Serialize the request.
    FFS_CHECK_RESULT(ffsDssSerializeComputeConfigurationDataRequest(
            &computeConfigurationDataRequest, bodyStream));

    return FFS_SUCCESS;
}

/** @brief Handle the "compute configuration data" HTTP response body.
 */
static FFS_RESULT ffsHandleComputeConfigurationDataHttpResponseBody(FfsStream_t *bodyStream,
        void *callbackDataPointer)
{
    FfsDssHttpCallbackData_t *callbackData = (FfsDssHttpCallbackData_t *) callbackDataPointer;
    FfsDssClientContext_t *dssClientContext = callbackData->dssClientContext;

    // Get the response callbacks.
    FfsDssComputeConfigurationDataReponseCallbacks_t *responseCallbacks =
            (FfsDssComputeConfigurationDataReponseCallbacks_t *) callbackData->operationCallbackDataPointer;

    // Validate the signature.
    FFS_CHECK_RESULT(ffsDssClientHandleBody(bodyStream, callbackDataPointer));

    // Initialize the root JSON object.
    FfsJsonValue_t rootJsonObject;
    FFS_CHECK_RESULT(ffsInitializeJsonObject(bodyStream, &rootJsonObject));

    // Parse the response.
    FfsDssComputeConfigurationDataResponse_t computeConfigurationDataResponse;
    FfsJsonValue_t configurationObjectValue;
    FFS_CHECK_RESULT(ffsDssDeserializeComputeConfigurationDataResponse(&rootJsonObject,
            &computeConfigurationDataResponse, &configurationObjectValue));

    // Is the registration token defined?
    if (computeConfigurationDataResponse.registrationDetails.registrationToken) {

        // Save the registration details.
        if (responseCallbacks->saveRegistrationDetailsCallback) {
            FFS_CHECK_RESULT(responseCallbacks->saveRegistrationDetailsCallback(dssClientContext->userContext,
                    &computeConfigurationDataResponse.registrationDetails));
        }
    }

    // Is there a configuration object and has the caller requested configuration?
    if (!ffsJsonValueIsEmpty(&configurationObjectValue) && responseCallbacks->saveConfigurationCallback) {

        // Iterate through the key/value pairs.
        for (bool isDone = false; !isDone;) {

            // Parse the next pair.
            FfsJsonField_t field;
            FFS_CHECK_RESULT(ffsParseJsonKeyValuePair(&configurationObjectValue, &field, &isDone));

            // Do we have a pair?
            if (!isDone) {

                // Convert the JSON value to a map value.
                FfsMapValue_t mapValue;
                FFS_CHECK_RESULT(ffsConvertJsonValueToMapEntry(&field.value, &mapValue));

                // Pass the configuration value to the client.
                FFS_RESULT result = responseCallbacks->saveConfigurationCallback(dssClientContext->userContext,
                        field.key, &mapValue);

                // Check the result, ignoring not implemented errors.
                if (result != FFS_NOT_IMPLEMENTED) {
                    FFS_CHECK_RESULT(result);
                } else {
                    ffsLogDebug("Ignoring unsupported configuration entry");
                }
            }
        }
    }

    return FFS_SUCCESS;
}
