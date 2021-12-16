/** @file ffs_dss_client.c
 *
 * @brief Device Setup Service client.
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
#include "ffs/common/ffs_crypto.h"
#include "ffs/common/ffs_configuration_map.h"
#include "ffs/common/ffs_http.h"
#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/compat/ffs_dss_client_compat.h"
#include "ffs/dss/ffs_dss_client.h"

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#define DSS_ENDPOINT_DEFAULT                    "dp-sps-na.amazon.com"

#define REDIRECT_LOCATION_HEADER_KEY            "Location"
#define DSS_SIGNATURE_HEADER_KEY                "x-amzn-dss-signature"

#define HTTP_STATUS_CODE_TEMPORARY_REDIRECT     (307)
#define HTTP_STATUS_CODE_PERMANENT_REDIRECT     (308)
#define HTTPS_URL_PREFIX                        "https://"

// Static function prototypes.
static FFS_RESULT ffsDssClientHttpExecute(FfsDssClientContext_t *dssClientContext,
        FfsHttpRequest_t *httpRequest, FfsDssHttpCallbackData_t *dssResponse);
static FFS_RESULT ffsDssClientSetDefaultHost(struct FfsUserContext_s *userContext,
        FfsStream_t *hostStream);
static FFS_RESULT ffsExtractHostString(FfsStream_t *redirectUrlStream, FfsUrl_t *redirectUrl);

/*
 * Initialize the Device Setup Service client.
 */
FFS_RESULT ffsDssClientInit(struct FfsUserContext_s *userContext, FfsDssClientContext_t *dssClientContext) {

    // Zero out the context.
    memset(dssClientContext, 0, sizeof(*dssClientContext));

    // Get the buffers.
    FFS_CHECK_RESULT(ffsDssClientGetBuffers(userContext,
            &(dssClientContext->hostStream),
            &(dssClientContext->sessionIdStream),
            &(dssClientContext->nonceStream),
            &(dssClientContext->bodyStream)));

    // Flush the streams.
    FFS_CHECK_RESULT(ffsFlushStream(&(dssClientContext->hostStream)));
    FFS_CHECK_RESULT(ffsFlushStream(&(dssClientContext->sessionIdStream)));
    FFS_CHECK_RESULT(ffsFlushStream(&(dssClientContext->nonceStream)));
    FFS_CHECK_RESULT(ffsFlushStream(&(dssClientContext->bodyStream)));

    // Write the default DSS host to the "host" entry.
    FFS_CHECK_RESULT(ffsDssClientGetDefaultHost(userContext, &(dssClientContext->hostStream), &(dssClientContext->port)));

    // Store the user context in the DSS context.
    dssClientContext->userContext = userContext;

    // Sequence numbers start from 1.
    dssClientContext->sequenceNumber = 1;

    return FFS_SUCCESS;
}

/*
 * Execute a request to the Device Setup Service.
 */
FFS_RESULT ffsDssClientExecute(FfsDssClientContext_t *dssClientContext,
        const FfsDssOperationData_t *dssOperation, FfsStream_t *bodyStream,
        void *callbackDataPointer)
{
    FFS_TEMPORARY_OUTPUT_STREAM(signatureStream, FFS_MAXIMUM_DER_SIGNATURE_SIZE);

    // Bump the sequence number.
    dssClientContext->sequenceNumber++;

    // Construct the HTTP request.
    FfsHttpHeader_t contentTypeHeader = {
        .nameStream = FFS_STRING_INPUT_STREAM(FFS_HTTP_HEADER_CONTENT_TYPE_NAME),
        .valueStream = FFS_STRING_INPUT_STREAM(FFS_HTTP_HEADER_CONTENT_TYPE_VALUE)
    };
    FfsHttpHeader_t *headers[] = {
        &contentTypeHeader,
        NULL
    };
    FfsHttpRequest_t httpRequest = {
        .operation = FFS_HTTP_OPERATION_POST,
        .url = {
            .scheme = FFS_HTTP_SCHEME_HTTPS,
            .port = (dssClientContext->connectedToSocksNetwork ? FFS_TRANSPARENT_SOCKS_PORT : dssClientContext->port),
            .hostStream = dssClientContext->hostStream,
            .path = dssOperation->path
        },
        .headers = headers,
        .bodyStream = *bodyStream,
        .callbacks = dssOperation->httpCallbacks
    };

    // Initialize the response.
    FfsDssHttpCallbackData_t dssResponse = {
        .dssClientContext = dssClientContext,
        .hasStatusCode = false,
        .hasSignature = false,
        .signatureStream = &signatureStream,
        .hasBody = false,
        .signatureIsVerified = false,
        .hasRedirect = false,
        .redirectUrl = &httpRequest.url,
        .operationCallbackDataPointer = callbackDataPointer,
        .result = FFS_SUCCESS
    };

    // Execute the request.
    FFS_CHECK_RESULT(ffsDssClientHttpExecute(dssClientContext, &httpRequest, &dssResponse));

    // Did a handler fail?
    FFS_CHECK_RESULT(dssResponse.result);

    // Did we fail to get a status code?
    if (!dssResponse.hasStatusCode) {
        ffsLogError("Failed to get a status code");
        FFS_FAIL(FFS_ERROR);
    }

    // Redirect?
    if (dssResponse.hasRedirect) {

        // Permanent redirect?
        if (dssResponse.statusCode == HTTP_STATUS_CODE_PERMANENT_REDIRECT) {

            // Save the host string.
            FFS_CHECK_RESULT(ffsDssClientSetDefaultHost(dssClientContext->userContext,
                    &dssClientContext->hostStream));
        }
    } else {

        // Did we get all the way through the state machine?
        if (!dssResponse.signatureIsVerified) {
            ffsLogError("Failed to verify signature");
            FFS_FAIL(FFS_ERROR);
        }
    }

    return FFS_SUCCESS;
}

/*
 * Handle a status code.
 */
FFS_RESULT ffsDssClientHandleStatusCode(int32_t statusCode, void *dssResponsePointer)
{
    FfsDssHttpCallbackData_t *dssResponse = (FfsDssHttpCallbackData_t *) dssResponsePointer;

    // Save it (overwriting any previous value).
    dssResponse->hasStatusCode = true;
    dssResponse->statusCode = statusCode;

    return FFS_SUCCESS;
}

/*
 * Handle a (possible redirect or signature) header.
 */
FFS_RESULT ffsDssClientHandleHeader(FfsStream_t *keyStream, FfsStream_t *valueStream,
        void *dssResponsePointer)
{
    FfsDssHttpCallbackData_t *dssResponse = (FfsDssHttpCallbackData_t *) dssResponsePointer;

    char keyString[FFS_STREAM_DATA_SIZE(*keyStream) + 1];
    char valueString[FFS_STREAM_DATA_SIZE(*valueStream) + 1];

    memcpy(keyString, FFS_STREAM_NEXT_READ(*keyStream), FFS_STREAM_DATA_SIZE(*keyStream));
    keyString[FFS_STREAM_DATA_SIZE(*keyStream)] = 0;
    memcpy(valueString, FFS_STREAM_NEXT_READ(*valueStream), FFS_STREAM_DATA_SIZE(*valueStream));
    valueString[FFS_STREAM_DATA_SIZE(*valueStream)] = 0;

    ffsLogDebug("Processing header: %s: %s", keyString, valueString);

    // Redirect location key?
    if (dssResponse->hasStatusCode && (dssResponse->statusCode == HTTP_STATUS_CODE_TEMPORARY_REDIRECT
            || dssResponse->statusCode == HTTP_STATUS_CODE_PERMANENT_REDIRECT)
            && ffsStreamMatchesString(keyStream, REDIRECT_LOCATION_HEADER_KEY)) {

        // Duplicate header?
        if (dssResponse->hasRedirect) {

            // Save the error.
            dssResponse->result = FFS_ERROR;

            // Fail the handler.
            FFS_FAIL(FFS_ERROR);
        }

        // Try to extract the host string.
        FFS_RESULT result = ffsExtractHostString(valueStream, dssResponse->redirectUrl);

        // Failed?
        if (result != FFS_SUCCESS) {

            // Save the error.
            dssResponse->result = result;

            // Fail the handler.
            FFS_FAIL(FFS_ERROR);
        }

        dssResponse->hasRedirect = true;
    }

    // Signature key?
    if (ffsStreamMatchesString(keyStream, DSS_SIGNATURE_HEADER_KEY)) {

        // Duplicate header?
        if (dssResponse->hasSignature) {

            // Save the error.
            dssResponse->result = FFS_ERROR;

            // Fail the handler.
            FFS_FAIL(FFS_ERROR);
        }

        // Try to deserialize the signature.
        FFS_RESULT result = ffsDecodeBase64(valueStream, dssResponse->signatureStream);

        // Failed?
        if (result != FFS_SUCCESS) {

            // Save the error.
            dssResponse->result = result;

            // Fail the handler.
            FFS_FAIL(FFS_ERROR);
        }

        dssResponse->hasSignature = true;
    }

    return FFS_SUCCESS;
}

/*
 * Handle the body (for signature validation).
 */
FFS_RESULT ffsDssClientHandleBody(FfsStream_t *bodyStream, void *dssResponsePointer)
{
    FfsDssHttpCallbackData_t *dssResponse = (FfsDssHttpCallbackData_t *) dssResponsePointer;

    // Is the signature missing or have we already processed a response body?
    if (!dssResponse->hasSignature || dssResponse->hasBody) {

        // Save the error.
        dssResponse->result = FFS_ERROR;

        // Fail the handler.
        FFS_FAIL(FFS_ERROR);
    }

    dssResponse->hasBody = true;

    // Try to verify the signature.
    FFS_RESULT result = ffsVerifyCloudSignature(dssResponse->dssClientContext->userContext,
            bodyStream, dssResponse->signatureStream, &dssResponse->signatureIsVerified);

    // Failed?
    if (result != FFS_SUCCESS) {

        // Save the error.
        dssResponse->result = result;

        // Fail the handler.
        FFS_FAIL(FFS_ERROR);
    }

    // Signature is invalid?
    if (!dssResponse->signatureIsVerified) {
        ffsLogError("Signature is invalid. Failing the call.");

        // Save the error.
        dssResponse->result = FFS_ERROR;

        // Fail the handler.
        FFS_FAIL(FFS_ERROR);
    }
    ffsLogDebug("Signature is valid.");
    return FFS_SUCCESS;
}

/*
 * Handle a redirect (temporary or permanent).
 */
FFS_RESULT ffsDssClientHandleRedirect(int32_t statusCode, FfsStream_t *locationStream,
        void *dssResponsePointer)
{
    (void) statusCode;
    (void) locationStream;
    (void) dssResponsePointer;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Reset the callback data before a retry.
 */
FFS_RESULT ffsDssClientBeforeRetry(void *dssResponsePointer)
{
    FfsDssHttpCallbackData_t *dssResponse = (FfsDssHttpCallbackData_t *) dssResponsePointer;

    dssResponse->hasStatusCode = false;
    dssResponse->hasSignature = false;
    dssResponse->hasBody = false;
    dssResponse->hasRedirect = false;
    dssResponse->result = FFS_SUCCESS;
    FFS_CHECK_RESULT(ffsFlushStream(dssResponse->signatureStream));

    return FFS_SUCCESS;
}

/*
 * Set the session ID for the Device Setup Service client.
 */
FFS_RESULT ffsDssClientSetSessionId(FfsDssClientContext_t *dssClientContext,
        const char *sessionId)
{
    FFS_CHECK_RESULT(ffsFlushStream(&dssClientContext->sessionIdStream));
    FFS_CHECK_RESULT(ffsWriteStringToStream(sessionId, &dssClientContext->sessionIdStream));
    FFS_CHECK_RESULT(ffsWriteByteToStream(0, &dssClientContext->sessionIdStream));

    return FFS_SUCCESS;
}

/** @brief Get the current Device Setup Service client session ID.
 *
 * @param dssClientContext DSS client context
 * @param sessionId Destination for the session ID string
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssClientGetSessionId(FfsDssClientContext_t *dssClientContext,
        const char **sessionId)
{
    if (FFS_STREAM_DATA_SIZE(dssClientContext->sessionIdStream) == 0) {
        *sessionId = NULL;
    } else {
        *sessionId = (const char *) FFS_STREAM_NEXT_READ(dssClientContext->sessionIdStream);
    }

    return FFS_SUCCESS;
}

/*
 * Refresh the nonce for the Device Setup Service client.
 */
FFS_RESULT ffsDssClientRefreshNonce(FfsDssClientContext_t *dssClientContext)
{
    FFS_CHECK_RESULT(ffsFlushStream(&dssClientContext->nonceStream));

    FFS_TEMPORARY_OUTPUT_STREAM(dataStream, 3);
    FFS_TEMPORARY_OUTPUT_STREAM(base64Stream, 4);

    // Generate the nonce 4 characters at a time.
    while(FFS_STREAM_SPACE_SIZE(dssClientContext->nonceStream) > 1) {

        // Get new characters?
        if (FFS_STREAM_DATA_SIZE(base64Stream) == 0) {

            // Get 3 random bytes.
            FFS_CHECK_RESULT(ffsFlushStream(&dataStream));
            FFS_CHECK_RESULT(ffsRandomBytes(dssClientContext->userContext, &dataStream));

            // Convert it to base-64.
            FFS_CHECK_RESULT(ffsFlushStream(&base64Stream));
            FFS_CHECK_RESULT(ffsEncodeBase64(&dataStream, 0, NULL, &base64Stream));
        }

        // Copy one character from the base-64 stream to the nonce.
        uint8_t *base64Character;
        FFS_CHECK_RESULT(ffsReadStream(&base64Stream, 1, &base64Character));
        FFS_CHECK_RESULT(ffsWriteByteToStream(*base64Character, &dssClientContext->nonceStream));
    }

    // Null-terminate the nonce.
    FFS_CHECK_RESULT(ffsWriteByteToStream(0, &dssClientContext->nonceStream));

    return FFS_SUCCESS;
}

/*
 * Get the current Device Setup Service client nonce.
 */
FFS_RESULT ffsDssClientGetNonce(FfsDssClientContext_t *dssClientContext, const char **nonce)
{
    *nonce = (const char *) FFS_STREAM_NEXT_READ(dssClientContext->nonceStream);

    return FFS_SUCCESS;
}

/*
 * Set the DSS host.
 */
static FFS_RESULT ffsDssClientSetDefaultHost(struct FfsUserContext_s *userContext,
        FfsStream_t *hostStream)
{
    FfsMapValue_t hostValue = {
        .type = FFS_MAP_VALUE_TYPE_STRING,
        .stringStream = *hostStream
    };
    FFS_RESULT result = ffsSetConfigurationValue(userContext, FFS_CONFIGURATION_ENTRY_KEY_DSS_HOST,
            &hostValue);

    // "Not implemented" is allowed (i.e., always start by hitting the default host).
    if (result != FFS_NOT_IMPLEMENTED) {
        FFS_CHECK_RESULT(result);
    }

    return FFS_SUCCESS;
}

/*
 * Get the DSS host.
 */
FFS_RESULT ffsDssClientGetDefaultHost(struct FfsUserContext_s *userContext,
        FfsStream_t *hostStream, uint16_t *port)
{
    FfsMapValue_t hostValue = {
        .stringStream = *hostStream
    };
    FfsMapValue_t portValue = {
        .stringStream = FFS_NULL_STREAM
    };

    FFS_CHECK_RESULT(ffsFlushStream(hostStream));

    // Try to get the configuration entry for the DSS host.
    FFS_RESULT result = ffsGetConfigurationValue(userContext, FFS_CONFIGURATION_ENTRY_KEY_DSS_HOST,
            &hostValue);

    switch (result) {
    case FFS_SUCCESS:

        // The entry should be a string.
        if (hostValue.type != FFS_MAP_VALUE_TYPE_STRING) {
            FFS_FAIL(FFS_ERROR);
        }

        // Update the host stream.
        *hostStream = hostValue.stringStream;
        break;

    case FFS_NOT_IMPLEMENTED:

        // No entry - use the default.
        FFS_CHECK_RESULT(ffsWriteStringToStream(DSS_ENDPOINT_DEFAULT, hostStream));
        break;

    default:

        // Fail.
        FFS_FAIL(result);
    }

    // Try to get the configuration entry for the DSS port.
    result = ffsGetConfigurationValue(userContext, FFS_CONFIGURATION_ENTRY_KEY_DSS_PORT,
            &portValue);

    switch (result) {
    case FFS_SUCCESS:

        // The entry should be a number.
        if (portValue.type != FFS_MAP_VALUE_TYPE_INTEGER) {
            FFS_FAIL(FFS_ERROR);
        }

        // Update the port;
        *port = (uint16_t) portValue.integerValue;
        break;

    case FFS_NOT_IMPLEMENTED:

        // No entry - use the default.
        *port = FFS_HTTPS_PORT;
        break;

    default:

        // Fail.
        FFS_FAIL(result);
    }


    return FFS_SUCCESS;
}

/*
 * Execute an HTTP request with redirects.
 */
static FFS_RESULT ffsDssClientHttpExecute(FfsDssClientContext_t *dssClientContext,
        FfsHttpRequest_t *httpRequest, FfsDssHttpCallbackData_t *dssResponse)
{
    for (int redirectCount = 0; redirectCount <= FFS_DSS_MAX_REDIRECTS; redirectCount++) {

        // Log the request URL.
        ffsLogDebug("DSS client sending request to: https://%.*s:%d%s",
                FFS_STREAM_DATA_SIZE(httpRequest->url.hostStream),
                FFS_STREAM_NEXT_READ(httpRequest->url.hostStream),
                httpRequest->url.port,
                httpRequest->url.path);

        // Use copies of the request and response to allow retries on redirect.
        FfsHttpRequest_t httpRequestCopy = *httpRequest;
        FfsDssHttpCallbackData_t dssResponseCopy = *dssResponse;

        // Execute the request.
        FFS_RESULT result = ffsHttpExecute(dssClientContext->userContext,
                &httpRequestCopy, &dssResponseCopy);

        // Log the returned status code.
        ffsLogDebug("DSS client received HTTP status code: %" PRId32, dssResponseCopy.statusCode);

        // Check the result.
        FFS_CHECK_RESULT(result);

        // Redirected?
        if (dssResponseCopy.hasRedirect) {

            // Log the redirect.
            ffsLogDebug("DSS client was redirected to: https://%.*s:%d%s",
                    FFS_STREAM_DATA_SIZE(dssResponseCopy.redirectUrl->hostStream),
                    FFS_STREAM_NEXT_READ(dssResponseCopy.redirectUrl->hostStream),
                    dssResponseCopy.redirectUrl->port,
                    dssResponseCopy.redirectUrl->path);
        } else {

            // Update the request and response.
            *httpRequest = httpRequestCopy;
            *dssResponse = dssResponseCopy;

            return FFS_SUCCESS;
        }
    }

    // Too many redirects.
    return FFS_ERROR;
}

/** @brief Extract the "host" component from a redirect target URL.
 *
 * Extract the "host" component from a redirect target URL of the form
 * https://{host}(:[0-9]+)?(/{path})?. The extracted "host" is
 * written to the corresponding field of the destination URL.
 *
 * @param redirectUrlStream Redirect target URL
 * @param redirectUrl Destination redirect URL
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT ffsExtractHostString(FfsStream_t *redirectUrlStream, FfsUrl_t *redirectUrl)
{
    // Skip the "https://" prefix.
    FFS_CHECK_RESULT(ffsReadExpected(redirectUrlStream, HTTPS_URL_PREFIX));

    // Flash the destination host stream.
    FFS_CHECK_RESULT(ffsFlushStream(&redirectUrl->hostStream));

    // Copy the URL one character at a time until we hit ":" or "/" or reach the end.
    while (!ffsStreamIsEmpty(redirectUrlStream)) {

        // Read the next character.
        uint8_t *character;
        FFS_CHECK_RESULT(ffsReadStream(redirectUrlStream, 1, &character));

        // Done?
        if (*character == ':' || *character == '/') {
            break;
        }

        // Write the character.
        FFS_CHECK_RESULT(ffsWriteByteToStream(*character, &redirectUrl->hostStream));
    }

    // TODO: update the port (if any) and validate that the path has not changed.
    return FFS_SUCCESS;
}
