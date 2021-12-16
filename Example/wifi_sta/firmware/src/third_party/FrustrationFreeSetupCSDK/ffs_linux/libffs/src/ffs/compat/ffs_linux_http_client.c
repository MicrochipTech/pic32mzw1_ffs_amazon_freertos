/** @file ffs_linux_http_client.c
 *
 * @brief Ffs libcurl HTTP client implementation.
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * This file is a Modifiable File, as defined in the accompanying LICENSE.TXT
 * file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_http.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"

#include <ctype.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

#define HEADER_LINE_SEPARATOR   ':'

/** @brief Macro to short-circuit curl functions and return a Ffs error.
 *
 * @param functionCall Function call returning @ref CURLcode
 */
#define FFS_HTTPCLIENT_CHECK_RESULT(functionCall) { \
        CURLcode curlCode = (functionCall); \
        if (curlCode != CURLE_OK) { \
            ffsLogError("Curl operation %s returned %d (%s)", \
                    #functionCall, curlCode, curl_easy_strerror(curlCode)); \
            FFS_FAIL(FFS_ERROR); \
        } \
    }

/** @brief Curl callback data.
 */
typedef struct {
    FfsHttpRequest_t *request; //!< Original request.
    void *callbackDataPointer; //!< Data for the request callbacks.
} FfsHttpClientCallbackData_t;

// Static function prototypes.
static FFS_RESULT ffsHttpExecutePreallocated(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer, CURL *session, struct curl_slist **headerList);
static FFS_RESULT ffsHttpConstructHeaderLine(FfsHttpHeader_t *header, char **headerLine);
static size_t ffsHttpHandleResponseHeader(char *buffer, size_t itemSize, size_t itemCount,
        FfsHttpClientCallbackData_t *httpClientCallbackData);
static size_t ffsHttpHandleResponseBody(char *buffer, size_t itemSize, size_t itemCount,
        FfsHttpClientCallbackData_t *httpClientCallbackData);
static FFS_RESULT ffsSetUrl(CURL *session, FfsHttpRequest_t *request);

/*
 * Execute an HTTP operation.
 */
FFS_RESULT ffsHttpExecute(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer)
{
    // Start curl.
    CURL *session = curl_easy_init();
    if (!session) {
        FFS_FAIL(FFS_ERROR);
    }

    curl_easy_setopt(session, CURLOPT_VERBOSE, 1L);

    // Header list.
    struct curl_slist *headerList = NULL;

    // Execute the operation.
    FFS_RESULT result = ffsHttpExecutePreallocated(userContext, request, callbackDataPointer, session,
            &headerList);

    // Clean up curl.
    curl_easy_cleanup(session);

    // Clean up the header list.
    if (headerList) {
        curl_slist_free_all(headerList);
    }

    // Check the result.
    FFS_CHECK_RESULT(result);

    return FFS_SUCCESS;
}

/*
 * Execute an HTTP operation with preallocated curl session and header list.
 */
static FFS_RESULT ffsHttpExecutePreallocated(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer, CURL *session, struct curl_slist **headerList)
{
    // Verify that the operation is a GET or a POST.
    if (request->operation != FFS_HTTP_OPERATION_GET &&
            request->operation != FFS_HTTP_OPERATION_POST) {
        FFS_FAIL(FFS_ERROR);
    }

    // Construct the HTTP client callback data.
    FfsHttpClientCallbackData_t httpClientCallbackData = {
        .request = request,
        .callbackDataPointer = callbackDataPointer
    };

    // Request a POST operation?
    if (request->operation == FFS_HTTP_OPERATION_POST) {
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_POST, 1L));
    }

    // Set the URL.
    FFS_CHECK_RESULT(ffsSetUrl(session, request));

    // Is the port defined?
    if (request->url.port > 0) {
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_PORT, request->url.port));
    }

    // Set the write callback.
    FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_WRITEFUNCTION,
            ffsHttpHandleResponseBody));
    FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_WRITEDATA, &httpClientCallbackData));

    // Set the header callback.
    FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_HEADERFUNCTION,
            ffsHttpHandleResponseHeader));
    FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_HEADERDATA, &httpClientCallbackData));

    // Check if the server CA certificates path is defined.
    if (userContext->serverCaCertificatesPath) {

        // Set the CA certificates path.
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_CAPATH,
                userContext->serverCaCertificatesPath));

        // Don't use the default CA info path.
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_CAINFO, NULL));

        // Verify the server certificate.
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_SSL_VERIFYPEER, 1L));

        // Verify that the host name is specified in the certificate.
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_SSL_VERIFYHOST, 2L));
    }

    // Check if the client certificate path is defined.
    if (userContext->clientCertificatePath) {
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_SSLCERT,
                userContext->clientCertificatePath));
    }

    // Check if the client certificate private key path is defined.
    if (userContext->clientCertificatePrivateKeyPath) {
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_SSLKEY,
                userContext->clientCertificatePrivateKeyPath));
    }

    // Are there any headers defined?
    if (request->headers) {

        // Iterate through the headers.
        for (FfsHttpHeader_t **header = request->headers; *header; header++) {

            // Add the header to the list.
            char *headerLine;
            FFS_CHECK_RESULT(ffsHttpConstructHeaderLine(*header, &headerLine));
            *headerList = curl_slist_append(*headerList, headerLine);
            free(headerLine);
        }

        // Set the headers.
        curl_easy_setopt(session, CURLOPT_HTTPHEADER, *headerList);
    }

    // Is a POST body defined?
    if (!ffsStreamIsEmpty(&(request->bodyStream))) {

        // Is this actually a POST?
        if (request->operation != FFS_HTTP_OPERATION_POST) {
            FFS_FAIL(FFS_ERROR);
        }

        ffsLogStream("POST body", &(request->bodyStream));

        // Set the POST body.
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_POSTFIELDSIZE,
                FFS_STREAM_DATA_SIZE(request->bodyStream)));
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_POSTFIELDS,
                FFS_STREAM_NEXT_READ(request->bodyStream)));
    }

    // Perform the operation.
    FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_perform(session));

    // Do we need to send the status code?
    if (request->callbacks.handleStatusCode) {

        // Get the status code.
        long statusCode;
        FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_getinfo(session, CURLINFO_RESPONSE_CODE, &statusCode));

        // Run the callback.
        FFS_CHECK_RESULT(request->callbacks.handleStatusCode((int32_t) statusCode, callbackDataPointer));
    }

    // Redirect?
    char *redirectUrl;
    FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_getinfo(session, CURLINFO_REDIRECT_URL, &redirectUrl));
    if (redirectUrl) {

        // TODO: add code to break the URL into components and execute the callback.
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/** @brief Construct and set the URL.
 */
static FFS_RESULT ffsSetUrl(CURL *session, FfsHttpRequest_t *request)
{
    // Get the length of the scheme.
    const char *scheme;
    switch (request->url.scheme) {
    case FFS_HTTP_SCHEME_HTTP:
        scheme = FFS_HTTP_SCHEME_STRING;
        break;
    case FFS_HTTP_SCHEME_HTTPS:
        scheme = FFS_HTTPS_SCHEME_STRING;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }
    size_t schemeLength = strlen(scheme);

    // Get the length of the host.
    size_t hostLength = FFS_STREAM_DATA_SIZE(request->url.hostStream);

    // The (maximum) length of the port.
    size_t portLength = 5;

    // Get the length of the path.
    size_t pathLength = 0;
    if (request->url.path) {
        pathLength = strlen(request->url.path);
    }

    // Verify that the required components are present.
    if (!schemeLength || !hostLength) {
        FFS_FAIL(FFS_ERROR);
    }

    // Temporary stream for storing the null-terminated URL.
    FFS_TEMPORARY_OUTPUT_STREAM(urlStream,
            schemeLength + 3 + hostLength + 1 + portLength + 1 + pathLength + 1);

    // Add the scheme.
    FFS_CHECK_RESULT(ffsWriteStringToStream(scheme, &urlStream));

    // Add the scheme/host separator.
    FFS_CHECK_RESULT(ffsWriteStringToStream("://", &urlStream));

    // Add the host (using a copy of the stream to leave the original unchanged).
    FfsStream_t hostStream = request->url.hostStream;
    FFS_CHECK_RESULT(ffsAppendStream(&hostStream, &urlStream));

    // Add the port, if present.
    if (request->url.port) {
        char portString[8];
        snprintf(portString, sizeof(portString), ":%d", (int) request->url.port);
        FFS_CHECK_RESULT(ffsWriteStringToStream(portString, &urlStream));
    }

    // Add the host/path separator.
    FFS_CHECK_RESULT(ffsWriteStringToStream("/", &urlStream));

    // Add the path if defined.
    if (request->url.path) {
        FFS_CHECK_RESULT(ffsWriteStringToStream(request->url.path, &urlStream));
    }

    // Terminate.
    FFS_CHECK_RESULT(ffsWriteByteToStream(0, &urlStream));

    // Set the URL.
    FFS_HTTPCLIENT_CHECK_RESULT(curl_easy_setopt(session, CURLOPT_URL,
            FFS_STREAM_NEXT_READ(urlStream)));

    return FFS_SUCCESS;
}

/** @brief Construct the full header line for the request.
 */
static FFS_RESULT ffsHttpConstructHeaderLine(FfsHttpHeader_t *header, char **headerLine)
{
    size_t nameLength = FFS_STREAM_DATA_SIZE(header->nameStream);
    if (nameLength == 0) {
        FFS_FAIL(FFS_ERROR);
    }

    size_t valueLength = FFS_STREAM_DATA_SIZE(header->valueStream);
    size_t separatorLength = 1;
    size_t totalLength = nameLength + separatorLength + valueLength;

    *headerLine = (char *) malloc(totalLength + 1);
    if (*headerLine == NULL) {
        FFS_FAIL(FFS_OVERRUN);
    }

    memcpy(*headerLine, FFS_STREAM_NEXT_READ(header->nameStream), nameLength);
    *(*headerLine + nameLength) = HEADER_LINE_SEPARATOR;
    memcpy(*headerLine + nameLength + separatorLength, FFS_STREAM_NEXT_READ(header->valueStream), valueLength);
    *(*headerLine + totalLength) = '\0';

    return FFS_SUCCESS;
}

/** @brief Callback to parse a response header.
 *
 * @param buffer Response header buffer.
 * @param itemSize Always 1
 * @param itemCount Size of data
 * @param httpClientCallbackData Callbacks
 *
 * @returns Size of the data processed or 0 on failure
 */
static size_t ffsHttpHandleResponseHeader(char *buffer, size_t itemSize, size_t itemCount,
        FfsHttpClientCallbackData_t *httpClientCallbackData)
{
    // Get the total size of the header data.
    size_t totalSize = itemCount * itemSize;

    // Is there a callback?
    if (httpClientCallbackData->request->callbacks.handleHeader) {

        // Parsing
        char *separator = memchr(buffer, HEADER_LINE_SEPARATOR, totalSize);
        if (!separator) {

            // Status line or custom header.
            return totalSize;
        }

        char *nameStart = buffer;
        while (nameStart < separator && isspace(*nameStart)) {
            nameStart++;
        }
        if (nameStart == separator) {

            // No name, whitespace only.
            return totalSize;
        }

        char *nameEnd = separator - 1;
        while (nameEnd >= nameStart && isspace(*nameEnd)) {
            nameEnd--;
        }

        char *valueStart = separator + 1;
        char *valueEnd = buffer + totalSize - 1;
        while (valueStart < buffer + totalSize && isspace(*valueStart)) {
            valueStart++;
        }
        if (valueStart == buffer + totalSize) {

            // No value, but that is allowed.
            valueEnd = valueStart;
        } else {
            while (valueEnd > separator && isspace(*valueEnd)) {
                valueEnd--;
            }
        }

        FfsStream_t nameStream = ffsCreateInputStream((uint8_t *) nameStart, nameEnd - nameStart + 1);
        FfsStream_t valueStream;
        if (valueStart != valueEnd) {
            valueStream = ffsCreateInputStream((uint8_t *) valueStart, valueEnd - valueStart + 1);
        }

        // Run the callback.
        FFS_RESULT result = httpClientCallbackData->request->callbacks.handleHeader(&nameStream, &valueStream,
                httpClientCallbackData->callbackDataPointer);

        // Error?
        if (result != FFS_SUCCESS) {
            return 0;
        }
    }

    return totalSize;
}

/** @brief Handle a response body.
 *
 * @param buffer Response data buffer.
 * @param itemSize Always 1
 * @param itemCount Size of data
 * @param httpClientCallbackData HTTP client callback data
 *
 * @returns Size of the data processed or 0 on failure
 */
static size_t ffsHttpHandleResponseBody(char *buffer, size_t itemSize, size_t itemCount,
        FfsHttpClientCallbackData_t *httpClientCallbackData)
{
    // Get the body size.
    size_t totalSize = itemCount * itemSize;

    // Is there a callback?
    if (httpClientCallbackData->request->callbacks.handleBody) {

        // Wrap the buffer.
        FfsStream_t bodyStream = ffsCreateInputStream((uint8_t *) buffer, totalSize);

        ffsLogStream("POST response body", &bodyStream);
        ffsLogDebug("%.*s", totalSize, buffer);

        // Run the callback.
        FFS_RESULT result = httpClientCallbackData->request->callbacks.handleBody(&bodyStream,
                httpClientCallbackData->callbackDataPointer);

        // Error?
        if (result != FFS_SUCCESS) {
            return 0;
        }
    }

    return totalSize;
}
