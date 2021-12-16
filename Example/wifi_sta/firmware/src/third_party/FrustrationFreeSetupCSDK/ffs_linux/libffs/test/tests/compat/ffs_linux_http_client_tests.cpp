/** @file ffs_linux_http_client_tests.cpp
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

#include "ffs/common/ffs_http.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"
#include "test_map.h"

#include "test_utilities.h"

#define TEST_VALID_HOST         ("www.example.com")
#define TEST_INVALID_HOST       ("****")

#define TEST_ECHO_GET_HOST      ("httpbin.org")
#define TEST_ECHO_GET_PATH      ("get")
#define TEST_ECHO_POST_HOST     ("httpbin.org")
#define TEST_ECHO_POST_PATH     ("post")

#define TEST_REQUEST_BODY       ("TEST")
#define RESPONSE_BODY_SZ        (2048)

#define TEST_PATH_BUFFER_SIZE   (1024)

#define TEST_CLIENT_CERTIFICATE                "data/client_certificate/certificate.pem"
#define TEST_CLIENT_CERTIFICATE_PRIVATE_KEY    "data/client_certificate/private_key.pem"
#define TEST_SERVER_CERTIFICATES_VALID         "data/server_certificates/"
#define TEST_SERVER_CERTIFICATES_INVALID       "data_invalid/"

/** @brief Assert that we got some 200 HTTP code back
 */
#define ASSERT_2XX(statusCode) ASSERT_EQ((statusCode) / 100, 2)

#define ZERO_FILL(variable) memset(&variable, 0, sizeof(variable))

/** @brief Test callback data.
 */
typedef struct {
    int32_t statusCode;
    FfsStream_t bodyStream;
} TestCallbackData_t;

/** @brief Save the status code.
 */
static FFS_RESULT handleStatusCode(int32_t statusCode, void *callbackDataPointer)
{
    TestCallbackData_t *testCallbackData = (TestCallbackData_t *) callbackDataPointer;

    testCallbackData->statusCode = statusCode;

    return FFS_SUCCESS;
}

/** @brief Save the response body.
 */
static FFS_RESULT handleBody(FfsStream_t *bodyStream, void *callbackDataPointer)
{
    TestCallbackData_t *testCallbackData = (TestCallbackData_t *) callbackDataPointer;

    testCallbackData->bodyStream = *bodyStream;

    return FFS_SUCCESS;
}

class HttpClientTests: public TestContextFixture {
};

/** @brief Test a GET operation to a valid URL.
 */
#if defined(BRAZIL)
TEST_F(HttpClientTests, DISABLED_Get)
#else
TEST_F(HttpClientTests, Get)
#endif
{
    // No certs.
    struct FfsUserContext_s userContext;
    ZERO_FILL(userContext);

    // Invalid HTTP.
    FfsUrl_t url;
    ZERO_FILL(url);
    url.scheme = FFS_HTTP_SCHEME_HTTP;
    url.hostStream = FFS_STRING_INPUT_STREAM(TEST_VALID_HOST);

    // Response body buffer.
    FFS_TEMPORARY_OUTPUT_STREAM(responseBodyStream, RESPONSE_BODY_SZ);

    // Callbacks.
    FfsHttpCallbacks_t callbacks;
    ZERO_FILL(callbacks);
    callbacks.handleStatusCode = handleStatusCode;
    callbacks.handleBody = handleBody;

    // Request.
    FfsHttpRequest_t request;
    ZERO_FILL(request);
    request.operation = FFS_HTTP_OPERATION_GET;
    request.url = url;
    request.bodyStream = responseBodyStream;
    request.callbacks = callbacks;

    // Empty callback data.
    TestCallbackData_t testCallbackData;
    ZERO_FILL(testCallbackData);

    // Execute the GET.
    ASSERT_EQ(ffsHttpExecute(&userContext, &request, &testCallbackData), FFS_SUCCESS);

    // Status code: 2xx.
    ASSERT_2XX(testCallbackData.statusCode);

    // Verify that we got some data back.
    ASSERT_FALSE(ffsStreamIsEmpty(&testCallbackData.bodyStream));
}

/** @brief Test a GET operation to an invalid URL.
 */
#if defined(BRAZIL)
TEST_F(HttpClientTests, DISABLED_GetBadHost)
#else
TEST_F(HttpClientTests, GetBadHost)
#endif
{
    // No certs.
    struct FfsUserContext_s userContext;
    ZERO_FILL(userContext);

    // Invalid HTTP.
    FfsUrl_t url;
    ZERO_FILL(url);
    url.scheme = FFS_HTTP_SCHEME_HTTP;
    url.hostStream = FFS_STRING_INPUT_STREAM(TEST_INVALID_HOST);

    // Response body buffer.
    FFS_TEMPORARY_OUTPUT_STREAM(responseBodyStream, RESPONSE_BODY_SZ);

    // Callbacks.
    FfsHttpCallbacks_t callbacks;
    ZERO_FILL(callbacks);
    callbacks.handleStatusCode = handleStatusCode;
    callbacks.handleBody = handleBody;

    // Request.
    FfsHttpRequest_t request;
    ZERO_FILL(request);
    request.operation = FFS_HTTP_OPERATION_GET;
    request.url = url;
    request.bodyStream = responseBodyStream;
    request.callbacks = callbacks;

    // Empty callback data.
    TestCallbackData_t testCallbackData;
    ZERO_FILL(testCallbackData);

    // Execute the GET.
    ASSERT_EQ(ffsHttpExecute(&userContext, &request, &testCallbackData), FFS_ERROR);
}

/** @brief Test a POST operation to a valid URL.
 */
#if defined(BRAZIL)
TEST_F(HttpClientTests, DISABLED_Post)
#else
TEST_F(HttpClientTests, Post)
#endif
{
    // No certs.
    struct FfsUserContext_s userContext;
    ZERO_FILL(userContext);

    // Valid HTTP.
    FfsUrl_t url;
    ZERO_FILL(url);
    url.scheme = FFS_HTTP_SCHEME_HTTP;
    url.hostStream = FFS_STRING_INPUT_STREAM(TEST_ECHO_POST_HOST);
    url.path = TEST_ECHO_POST_PATH;

    // Request/response body buffer.
    FFS_TEMPORARY_OUTPUT_STREAM(responseBodyStream, RESPONSE_BODY_SZ);

    // Callbacks.
    FfsHttpCallbacks_t callbacks;
    ZERO_FILL(callbacks);
    callbacks.handleStatusCode = handleStatusCode;
    callbacks.handleBody = handleBody;

    // Request.
    FfsHttpRequest_t request;
    ZERO_FILL(request);
    request.operation = FFS_HTTP_OPERATION_POST;
    request.url = url;
    request.bodyStream = responseBodyStream;
    request.callbacks = callbacks;

    // Set the request body.
    ffsWriteStringToStream(TEST_REQUEST_BODY, &request.bodyStream);

    // Empty callback data.
    TestCallbackData_t testCallbackData;
    ZERO_FILL(testCallbackData);

    // Execute the POST.
    ASSERT_EQ(ffsHttpExecute(&userContext, &request, &testCallbackData), FFS_SUCCESS);

    // Status code: 2xx.
    ASSERT_2XX(testCallbackData.statusCode);

    // Verify that we got some data back.
    ASSERT_FALSE(ffsStreamIsEmpty(&testCallbackData.bodyStream));
}

/** @brief Test a POST operation with server certificate validation and a valid certificate.
 */
#if defined(__APPLE__) || defined(BRAZIL)
TEST_F(HttpClientTests, DISABLED_PostServerCertificateValid)
#else
TEST_F(HttpClientTests, PostServerCertificateValid)
#endif
{
    // Construct the server certs path.
    char serverCaCertificatesPath[TEST_PATH_BUFFER_SIZE];
    constructTestPath(TEST_SERVER_CERTIFICATES_VALID, serverCaCertificatesPath, sizeof(serverCaCertificatesPath));

    // Servers certs only.
    struct FfsUserContext_s userContext;
    ZERO_FILL(userContext);
    userContext.serverCaCertificatesPath = serverCaCertificatesPath;

    // Valid HTTPS.
    FfsUrl_t url;
    ZERO_FILL(url);
    url.scheme = FFS_HTTP_SCHEME_HTTPS;
    url.hostStream = FFS_STRING_INPUT_STREAM(TEST_ECHO_POST_HOST);
    url.path = TEST_ECHO_POST_PATH;

    // Request/response body buffer.
    FFS_TEMPORARY_OUTPUT_STREAM(responseBodyStream, RESPONSE_BODY_SZ);

    // Callbacks.
    FfsHttpCallbacks_t callbacks;
    ZERO_FILL(callbacks);
    callbacks.handleStatusCode = handleStatusCode;
    callbacks.handleBody = handleBody;

    // Request.
    FfsHttpRequest_t request;
    ZERO_FILL(request);
    request.operation = FFS_HTTP_OPERATION_POST;
    request.url = url;
    request.bodyStream = responseBodyStream;
    request.callbacks = callbacks;

    // Set the request body.
    ffsWriteStringToStream(TEST_REQUEST_BODY, &request.bodyStream);

    // Empty callback data.
    TestCallbackData_t testCallbackData;
    ZERO_FILL(testCallbackData);

    // Execute the GET.
    ASSERT_EQ(ffsHttpExecute(&userContext, &request, &testCallbackData), FFS_SUCCESS);

    // Status code: 2xx.
    ASSERT_2XX(testCallbackData.statusCode);

    // Verify that we got some data back.
    ASSERT_FALSE(ffsStreamIsEmpty(&testCallbackData.bodyStream));
}

/** @brief Test a POST operation with server certificate validation and invalid certificate.
 */
#if defined(__APPLE__) || defined(BRAZIL)
TEST_F(HttpClientTests, DISABLED_PostServerCertificateInvalid)
#else
TEST_F(HttpClientTests, PostServerCertificateInvalid)
#endif
{
    // Construct the server certs path.
    char serverCaCertificatesPath[TEST_PATH_BUFFER_SIZE];
    constructTestPath(TEST_SERVER_CERTIFICATES_INVALID, serverCaCertificatesPath, sizeof(serverCaCertificatesPath));

    // Servers certs only.
    struct FfsUserContext_s userContext;
    ZERO_FILL(userContext);
    userContext.serverCaCertificatesPath = serverCaCertificatesPath;

    // Valid HTTPS.
    FfsUrl_t url;
    ZERO_FILL(url);
    url.scheme = FFS_HTTP_SCHEME_HTTPS;
    url.hostStream = FFS_STRING_INPUT_STREAM(TEST_ECHO_POST_HOST);
    url.path = TEST_ECHO_POST_PATH;

    // Request/response body buffer.
    FFS_TEMPORARY_OUTPUT_STREAM(responseBodyStream, RESPONSE_BODY_SZ);

    // Request.
    FfsHttpRequest_t request;
    ZERO_FILL(request);
    request.operation = FFS_HTTP_OPERATION_POST;
    request.url = url;
    request.bodyStream = responseBodyStream;

    // Set the request body.
    ffsWriteStringToStream(TEST_REQUEST_BODY, &request.bodyStream);

    // Empty callback data.
    TestCallbackData_t testCallbackData;
    ZERO_FILL(testCallbackData);

    // Execute the GET.
    ASSERT_EQ(ffsHttpExecute(&userContext, &request, &testCallbackData), FFS_ERROR);
}

/** @brief Test a GET operation with client certificate.
 *
 * Note: The server is currently not actually validating the client
 * certificate. This test only verifies the variable
 * names/parsing/no crashing.
 */
#if defined(__APPLE__) || defined(BRAZIL)
TEST_F(HttpClientTests, DISABLED_GetClientCertificate)
#else
TEST_F(HttpClientTests, GetClientCertificate)
#endif
{
    // Construct the client certs paths.
    char clientCertificatePath[TEST_PATH_BUFFER_SIZE];
    char clientCertificatePrivateKeyPath[TEST_PATH_BUFFER_SIZE];
    constructTestPath(TEST_CLIENT_CERTIFICATE, clientCertificatePath, sizeof(clientCertificatePath));
    constructTestPath(TEST_CLIENT_CERTIFICATE_PRIVATE_KEY, clientCertificatePrivateKeyPath, sizeof(clientCertificatePrivateKeyPath));

    // Client certs only.
    struct FfsUserContext_s userContext;
    ZERO_FILL(userContext);
    userContext.clientCertificatePath = clientCertificatePath;
    userContext.clientCertificatePrivateKeyPath = clientCertificatePrivateKeyPath;

    // Valid HTTPS.
    FfsUrl_t url;
    ZERO_FILL(url);
    url.scheme = FFS_HTTP_SCHEME_HTTPS;
    url.hostStream = FFS_STRING_INPUT_STREAM(TEST_ECHO_GET_HOST);
    url.path = TEST_ECHO_GET_PATH;

    // Request/response body buffer.
    FFS_TEMPORARY_OUTPUT_STREAM(responseBodyStream, RESPONSE_BODY_SZ);

    // Callbacks.
    FfsHttpCallbacks_t callbacks;
    ZERO_FILL(callbacks);
    callbacks.handleStatusCode = handleStatusCode;
    callbacks.handleBody = handleBody;

    // Request.
    FfsHttpRequest_t request;
    ZERO_FILL(request);
    request.operation = FFS_HTTP_OPERATION_GET;
    request.url = url;
    request.bodyStream = responseBodyStream;
    request.callbacks = callbacks;

    // Empty callback data.
    TestCallbackData_t testCallbackData;
    ZERO_FILL(testCallbackData);

    // Execute the GET.
    ASSERT_EQ(ffsHttpExecute(&userContext, &request, &testCallbackData), FFS_SUCCESS);

    // Status code: 2xx.
    ASSERT_2XX(testCallbackData.statusCode);

    // Verify that we got some data back.
    ASSERT_FALSE(ffsStreamIsEmpty(&testCallbackData.bodyStream));
}

/** @brief Test a GET operation with mutual authentication
 *
 * Note: The server is currently not actually validating the client
 * certificate. This test only verifies the variable
 * names/parsing/no crashing.
 */
#if defined(__APPLE__) || defined(BRAZIL)
TEST_F(HttpClientTests, DISABLED_GetMutualAuthentication)
#else
TEST_F(HttpClientTests, GetMutualAuthentication)
#endif
{
    // Construct the server and client certs paths.
    char serverCaCertificatesPath[TEST_PATH_BUFFER_SIZE];
    char clientCertificatePath[TEST_PATH_BUFFER_SIZE];
    char clientCertificatePrivateKeyPath[TEST_PATH_BUFFER_SIZE];
    constructTestPath(TEST_SERVER_CERTIFICATES_VALID, serverCaCertificatesPath, sizeof(serverCaCertificatesPath));
    constructTestPath(TEST_CLIENT_CERTIFICATE, clientCertificatePath, sizeof(clientCertificatePath));
    constructTestPath(TEST_CLIENT_CERTIFICATE_PRIVATE_KEY, clientCertificatePrivateKeyPath, sizeof(clientCertificatePrivateKeyPath));

    // Server and client certs.
    struct FfsUserContext_s userContext;
    ZERO_FILL(userContext);
    userContext.serverCaCertificatesPath = serverCaCertificatesPath;
    userContext.clientCertificatePath = clientCertificatePath;
    userContext.clientCertificatePrivateKeyPath = clientCertificatePrivateKeyPath;

    // Valid HTTPS.
    FfsUrl_t url;
    ZERO_FILL(url);
    url.scheme = FFS_HTTP_SCHEME_HTTPS;
    url.hostStream = FFS_STRING_INPUT_STREAM(TEST_ECHO_GET_HOST);
    url.path = TEST_ECHO_GET_PATH;

    // Request/response body buffer.
    FFS_TEMPORARY_OUTPUT_STREAM(responseBodyStream, RESPONSE_BODY_SZ);

    // Callbacks.
    FfsHttpCallbacks_t callbacks;
    ZERO_FILL(callbacks);
    callbacks.handleStatusCode = handleStatusCode;
    callbacks.handleBody = handleBody;

    // Request.
    FfsHttpRequest_t request;
    ZERO_FILL(request);
    request.operation = FFS_HTTP_OPERATION_GET;
    request.url = url;
    request.bodyStream = responseBodyStream;
    request.callbacks = callbacks;

    // Empty callback data.
    TestCallbackData_t testCallbackData;
    ZERO_FILL(testCallbackData);

    // Execute the GET.
    ASSERT_EQ(ffsHttpExecute(&userContext, &request, &testCallbackData), FFS_SUCCESS);

    // Status code: 2xx.
    ASSERT_2XX(testCallbackData.statusCode);

    // Verify that we got some data back.
    ASSERT_FALSE(ffsStreamIsEmpty(&testCallbackData.bodyStream));
}
