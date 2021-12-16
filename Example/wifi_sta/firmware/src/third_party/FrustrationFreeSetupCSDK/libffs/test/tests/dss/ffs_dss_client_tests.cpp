/** @file ffs_json_tests.cpp
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

#include "helpers/test_utilities.h"
#include "ffs/dss/ffs_dss_client.h"

#define HTTP_OK                 (200)
#define SIGNATURE_HEADER_KEY    "x-amzn-dss-signature"

/** @brief "Request has specified body" matcher.
 */
MATCHER_P(RequestBodyMatches, sourceRequestBody, "Request body matches")
{
    return arraysAreEqual(FFS_STREAM_NEXT_READ(arg->bodyStream),
            FFS_STREAM_DATA_SIZE(arg->bodyStream),
            (uint8_t *) sourceRequestBody,
            strlen(sourceRequestBody));
}

/** @brief "Response has specified operation data" matcher.
 */
MATCHER_P(ResponseOperationDataMatches, sourceOperationDataPointer, "Response operation data matches")
{
    return ((FfsDssHttpCallbackData_t *) arg)->operationCallbackDataPointer == sourceOperationDataPointer;
}

/** @brief Execute the "handle status code" callback.
 */
ACTION_P(ExecuteHandleStatusCodeCallback, statusCode)
{
    ((FfsHttpRequest_t *) std::get<1>(args))->callbacks.handleStatusCode(statusCode, std::get<2>(args));
}

/** @brief Execute the "handle header" callback.
 */
ACTION_P2(ExecuteHandleHeaderCallback, keyStream, valueStream)
{
    ((FfsHttpRequest_t *) std::get<1>(args))->callbacks.handleHeader(keyStream, valueStream, std::get<2>(args));
}

/** @brief Execute the "handle body" callback.
 */
ACTION_P(ExecuteHandleBodyCallback, bodyStream)
{
    ((FfsHttpRequest_t *) std::get<1>(args))->callbacks.handleBody(bodyStream, std::get<2>(args));
}

/** @brief Execute the "before retry" callback.
 */
ACTION(ExecuteBeforeRetryCallback)
{
    ((FfsHttpRequest_t *) std::get<1>(args))->callbacks.beforeRetry(std::get<2>(args));
}

class DssClientTests : public TestContextFixture {};

/*
 * Test DSS client initialization.
 */
TEST_F(DssClientTests, Initialization)
{
    FfsDssClientContext_t dssClientContext;

    // Set the expectations.
    FFS_TEMPORARY_OUTPUT_STREAM(hostStream, 100);
    FFS_TEMPORARY_OUTPUT_STREAM(sessionIdStream, 100);
    FFS_TEMPORARY_OUTPUT_STREAM(nonceStream, 100);
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, 100);
    EXPECT_COMPAT_CALL(ffsDssClientGetBuffers(getUserContext(), _, _, _, _))
                .WillOnce(DoAll(SetArgPointee<1>(hostStream),
                        SetArgPointee<2>(sessionIdStream),
                        SetArgPointee<3>(nonceStream),
                        SetArgPointee<4>(bodyStream), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(getUserContext(), FFS_CONFIGURATION_ENTRY_KEY_DSS_HOST, _))
                .WillOnce(Return(FFS_NOT_IMPLEMENTED));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(getUserContext(), FFS_CONFIGURATION_ENTRY_KEY_DSS_PORT, _))
                .WillOnce(Return(FFS_NOT_IMPLEMENTED));

    ASSERT_SUCCESS(ffsDssClientInit(getUserContext(), &dssClientContext));

    // Validate.
    ASSERT_EQ(FFS_STREAM_BUFFER(hostStream), FFS_STREAM_BUFFER(dssClientContext.hostStream));
    ASSERT_EQ(FFS_STREAM_BUFFER(sessionIdStream), FFS_STREAM_BUFFER(dssClientContext.sessionIdStream));
    ASSERT_EQ(FFS_STREAM_BUFFER(nonceStream), FFS_STREAM_BUFFER(dssClientContext.nonceStream));
    ASSERT_EQ(FFS_STREAM_BUFFER(bodyStream), FFS_STREAM_BUFFER(dssClientContext.bodyStream));
    ASSERT_EQ(1, dssClientContext.sequenceNumber);
    ASSERT_EQ(getUserContext(), dssClientContext.userContext);
}

/*
 * Test a DSS client request.
 */
TEST_F(DssClientTests, HttpRequest)
{
    const char *REQUEST_BODY = "{\"type\":\"REQUEST\"}";
    const char *RESPONSE_BODY = "{\"type\":\"RESPONSE\"}";
    const char *SIGNATURE_HEADER_VALUE = "U0lHTkFUVVJF";
    const char *SIGNATURE = "SIGNATURE";

    FfsDssOperationData_t dssOperation = {
        FFS_DSS_OPERATION_ID_REPORT,
        .name = "TEST",
        .path = "/test",
        {
            ffsDssClientHandleStatusCode,
            ffsDssClientHandleHeader,
            ffsDssClientHandleBody,
            ffsDssClientHandleRedirect,
            ffsDssClientBeforeRetry
        }
    };

    FfsStream_t requestBodyStream = FFS_STRING_INPUT_STREAM(REQUEST_BODY);
    FfsStream_t responseBodyStream = FFS_STRING_INPUT_STREAM(RESPONSE_BODY);
    FfsStream_t signatureHeaderKeyStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_KEY);
    FfsStream_t signatureHeaderValueStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_VALUE);

    int operationData = 0;

    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), RequestBodyMatches(REQUEST_BODY),
            ResponseOperationDataMatches(&operationData)))
            .WillOnce(DoAll(ExecuteHandleStatusCodeCallback(HTTP_OK),
                    ExecuteHandleHeaderCallback(&signatureHeaderKeyStream, &signatureHeaderValueStream),
                    ExecuteHandleBodyCallback(&responseBodyStream),
                    Return(FFS_SUCCESS)));

    EXPECT_COMPAT_CALL(ffsVerifyCloudSignature(getUserContext(), PointeeStreamEqString(RESPONSE_BODY),
            PointeeStreamEqString(SIGNATURE), _))
            .WillOnce(DoAll(SetArgPointee<3>(true), Return(FFS_SUCCESS)));

    ASSERT_SUCCESS(ffsDssClientExecute(getDssClientContext(), &dssOperation, &requestBodyStream,
            &operationData));
}

/*
 * Test an invalid signature.
 */
TEST_F(DssClientTests, InvalidSignature)
{
    const char *REQUEST_BODY = "{\"type\":\"REQUEST\"}";
    const char *RESPONSE_BODY = "{\"type\":\"RESPONSE\"}";
    const char *SIGNATURE_HEADER_VALUE = "U0lHTkFUVVJF";
    const char *SIGNATURE = "SIGNATURE";

    FfsDssOperationData_t dssOperation = {
        FFS_DSS_OPERATION_ID_REPORT,
        .name = "TEST",
        .path = "/test",
        {
            ffsDssClientHandleStatusCode,
            ffsDssClientHandleHeader,
            ffsDssClientHandleBody,
            ffsDssClientHandleRedirect,
            ffsDssClientBeforeRetry
        }
    };

    FfsStream_t requestBodyStream = FFS_STRING_INPUT_STREAM(REQUEST_BODY);
    FfsStream_t responseBodyStream = FFS_STRING_INPUT_STREAM(RESPONSE_BODY);
    FfsStream_t signatureHeaderKeyStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_KEY);
    FfsStream_t signatureHeaderValueStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_VALUE);

    int operationData = 1;

    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), RequestBodyMatches(REQUEST_BODY),
            ResponseOperationDataMatches(&operationData)))
            .WillOnce(DoAll(ExecuteHandleStatusCodeCallback(HTTP_OK),
                    ExecuteHandleHeaderCallback(&signatureHeaderKeyStream, &signatureHeaderValueStream),
                    ExecuteHandleBodyCallback(&responseBodyStream),
                    Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsVerifyCloudSignature(getUserContext(), PointeeStreamEqString(RESPONSE_BODY),
            PointeeStreamEqString(SIGNATURE), _))
            .WillOnce(DoAll(SetArgPointee<3>(false), Return(FFS_SUCCESS)));

    ASSERT_FAILURE(ffsDssClientExecute(getDssClientContext(), &dssOperation, &requestBodyStream,
            &operationData));
}

/*
 * Test a non-base64 signature.
 */
TEST_F(DssClientTests, SignatureNotBase64)
{
    const char *REQUEST_BODY = "{\"type\":\"REQUEST\"}";
    const char *RESPONSE_BODY = "{\"type\":\"RESPONSE\"}";
    const char *SIGNATURE_HEADER_VALUE = "U0lHTkFUVVJF===";

    FfsDssOperationData_t dssOperation = {
        FFS_DSS_OPERATION_ID_REPORT,
        .name = "TEST",
        .path = "/test",
        {
            ffsDssClientHandleStatusCode,
            ffsDssClientHandleHeader,
            ffsDssClientHandleBody,
            ffsDssClientHandleRedirect,
            ffsDssClientBeforeRetry
        }
    };

    FfsStream_t requestBodyStream = FFS_STRING_INPUT_STREAM(REQUEST_BODY);
    FfsStream_t responseBodyStream = FFS_STRING_INPUT_STREAM(RESPONSE_BODY);
    FfsStream_t signatureHeaderKeyStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_KEY);
    FfsStream_t signatureHeaderValueStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_VALUE);

    int operationData = 2;

    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), RequestBodyMatches(REQUEST_BODY),
            ResponseOperationDataMatches(&operationData)))
            .WillOnce(DoAll(ExecuteHandleStatusCodeCallback(HTTP_OK),
                    ExecuteHandleHeaderCallback(&signatureHeaderKeyStream, &signatureHeaderValueStream),
                    ExecuteHandleBodyCallback(&responseBodyStream),
                    Return(FFS_SUCCESS)));

    ASSERT_FAILURE(ffsDssClientExecute(getDssClientContext(), &dssOperation, &requestBodyStream,
            &operationData));
}

/*
 * Test a failed signature verification.
 */
TEST_F(DssClientTests, FailedSignatureVerification)
{
    const char *REQUEST_BODY = "{\"type\":\"REQUEST\"}";
    const char *RESPONSE_BODY = "{\"type\":\"RESPONSE\"}";
    const char *SIGNATURE_HEADER_VALUE = "U0lHTkFUVVJF";
    const char *SIGNATURE = "SIGNATURE";

    FfsDssOperationData_t dssOperation = {
        FFS_DSS_OPERATION_ID_REPORT,
        .name = "TEST",
        .path = "/test",
        {
            ffsDssClientHandleStatusCode,
            ffsDssClientHandleHeader,
            ffsDssClientHandleBody,
            ffsDssClientHandleRedirect,
            ffsDssClientBeforeRetry
        }
    };

    FfsStream_t requestBodyStream = FFS_STRING_INPUT_STREAM(REQUEST_BODY);
    FfsStream_t responseBodyStream = FFS_STRING_INPUT_STREAM(RESPONSE_BODY);
    FfsStream_t signatureHeaderKeyStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_KEY);
    FfsStream_t signatureHeaderValueStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_VALUE);

    int operationData = 1;

    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), RequestBodyMatches(REQUEST_BODY),
            ResponseOperationDataMatches(&operationData)))
            .WillOnce(DoAll(ExecuteHandleStatusCodeCallback(HTTP_OK),
                    ExecuteHandleHeaderCallback(&signatureHeaderKeyStream, &signatureHeaderValueStream),
                    ExecuteHandleBodyCallback(&responseBodyStream),
                    Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsVerifyCloudSignature(getUserContext(), PointeeStreamEqString(RESPONSE_BODY),
            PointeeStreamEqString(SIGNATURE), _))
            .WillOnce(DoAll(SetArgPointee<3>(true), Return(FFS_ERROR)));

    ASSERT_FAILURE(ffsDssClientExecute(getDssClientContext(), &dssOperation, &requestBodyStream,
            &operationData));
}

/*
 * Test a missing status code.
 */
TEST_F(DssClientTests, MissingStatusCode)
{
    const char *REQUEST_BODY = "{\"type\":\"REQUEST\"}";
    const char *RESPONSE_BODY = "{\"type\":\"RESPONSE\"}";
    const char *SIGNATURE_HEADER_VALUE = "U0lHTkFUVVJF";
    const char *SIGNATURE = "SIGNATURE";

    FfsDssOperationData_t dssOperation = {
        FFS_DSS_OPERATION_ID_REPORT,
        .name = "TEST",
        .path = "/test",
        {
            ffsDssClientHandleStatusCode,
            ffsDssClientHandleHeader,
            ffsDssClientHandleBody,
            ffsDssClientHandleRedirect,
            ffsDssClientBeforeRetry
        }
    };

    FfsStream_t requestBodyStream = FFS_STRING_INPUT_STREAM(REQUEST_BODY);
    FfsStream_t responseBodyStream = FFS_STRING_INPUT_STREAM(RESPONSE_BODY);
    FfsStream_t signatureHeaderKeyStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_KEY);
    FfsStream_t signatureHeaderValueStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_VALUE);

    int operationData = 2;

    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), RequestBodyMatches(REQUEST_BODY),
            ResponseOperationDataMatches(&operationData)))
            .WillOnce(DoAll(ExecuteHandleHeaderCallback(&signatureHeaderKeyStream, &signatureHeaderValueStream),
                    ExecuteHandleBodyCallback(&responseBodyStream),
                    Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsVerifyCloudSignature(getUserContext(), PointeeStreamEqString(RESPONSE_BODY),
            PointeeStreamEqString(SIGNATURE), _))
            .WillOnce(DoAll(SetArgPointee<3>(true), Return(FFS_SUCCESS)));

    ASSERT_FAILURE(ffsDssClientExecute(getDssClientContext(), &dssOperation, &requestBodyStream,
            &operationData));
}

/*
 * Test calling the signature header handler twice.
 */
TEST_F(DssClientTests, DuplicateSignature)
{
    const char *REQUEST_BODY = "{\"type\":\"REQUEST\"}";
    const char *RESPONSE_BODY = "{\"type\":\"RESPONSE\"}";
    const char *SIGNATURE_HEADER_VALUE = "U0lHTkFUVVJF";
    const char *SIGNATURE = "SIGNATURE";

    FfsDssOperationData_t dssOperation = {
        FFS_DSS_OPERATION_ID_REPORT,
        .name = "TEST",
        .path = "/test",
        {
            ffsDssClientHandleStatusCode,
            ffsDssClientHandleHeader,
            ffsDssClientHandleBody,
            ffsDssClientHandleRedirect,
            ffsDssClientBeforeRetry
        }
    };

    FfsStream_t requestBodyStream = FFS_STRING_INPUT_STREAM(REQUEST_BODY);
    FfsStream_t responseBodyStream = FFS_STRING_INPUT_STREAM(RESPONSE_BODY);
    FfsStream_t signatureHeaderKeyStream1 = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_KEY);
    FfsStream_t signatureHeaderValueStream1 = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_VALUE);
    FfsStream_t signatureHeaderKeyStream2 = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_KEY);
    FfsStream_t signatureHeaderValueStream2 = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_VALUE);

    int operationData = 3;

    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), RequestBodyMatches(REQUEST_BODY),
            ResponseOperationDataMatches(&operationData)))
            .WillOnce(DoAll(ExecuteHandleStatusCodeCallback(HTTP_OK),
                    ExecuteHandleHeaderCallback(&signatureHeaderKeyStream1, &signatureHeaderValueStream1),
                    ExecuteHandleHeaderCallback(&signatureHeaderKeyStream2, &signatureHeaderValueStream2),
                    ExecuteHandleBodyCallback(&responseBodyStream),
                    Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsVerifyCloudSignature(getUserContext(), PointeeStreamEqString(RESPONSE_BODY),
            PointeeStreamEqString(SIGNATURE), _))
            .WillOnce(DoAll(SetArgPointee<3>(true), Return(FFS_SUCCESS)));

    ASSERT_FAILURE(ffsDssClientExecute(getDssClientContext(), &dssOperation, &requestBodyStream,
            &operationData));
}

/*
 * Test calling the response body handler twice.
 */
TEST_F(DssClientTests, DuplicateBody)
{
    const char *REQUEST_BODY = "{\"type\":\"REQUEST\"}";
    const char *RESPONSE_BODY = "{\"type\":\"RESPONSE\"}";
    const char *SIGNATURE_HEADER_VALUE = "U0lHTkFUVVJF";
    const char *SIGNATURE = "SIGNATURE";

    FfsDssOperationData_t dssOperation = {
        FFS_DSS_OPERATION_ID_REPORT,
        .name = "TEST",
        .path = "/test",
        {
            ffsDssClientHandleStatusCode,
            ffsDssClientHandleHeader,
            ffsDssClientHandleBody,
            ffsDssClientHandleRedirect,
            ffsDssClientBeforeRetry
        }
    };

    FfsStream_t requestBodyStream = FFS_STRING_INPUT_STREAM(REQUEST_BODY);
    FfsStream_t responseBodyStream1 = FFS_STRING_INPUT_STREAM(RESPONSE_BODY);
    FfsStream_t responseBodyStream2 = FFS_STRING_INPUT_STREAM(RESPONSE_BODY);
    FfsStream_t signatureHeaderKeyStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_KEY);
    FfsStream_t signatureHeaderValueStream = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_VALUE);

    int operationData = 5;

    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), RequestBodyMatches(REQUEST_BODY),
            ResponseOperationDataMatches(&operationData)))
            .WillOnce(DoAll(ExecuteHandleStatusCodeCallback(HTTP_OK),
                    ExecuteHandleHeaderCallback(&signatureHeaderKeyStream, &signatureHeaderValueStream),
                    ExecuteHandleBodyCallback(&responseBodyStream1),
                    ExecuteHandleBodyCallback(&responseBodyStream2),
                    Return(FFS_SUCCESS)));

    EXPECT_COMPAT_CALL(ffsVerifyCloudSignature(getUserContext(), PointeeStreamEqString(RESPONSE_BODY),
            PointeeStreamEqString(SIGNATURE), _))
            .WillOnce(DoAll(SetArgPointee<3>(true), Return(FFS_SUCCESS)));

    ASSERT_FAILURE(ffsDssClientExecute(getDssClientContext(), &dssOperation, &requestBodyStream,
            &operationData));
}

TEST_F(DssClientTests, Retry)
{
    const char *REQUEST_BODY = "{\"type\":\"REQUEST\"}";
    const char *RESPONSE_BODY = "{\"type\":\"RESPONSE\"}";
    const char *SIGNATURE_HEADER_VALUE = "U0lHTkFUVVJF";
    const char *SIGNATURE = "SIGNATURE";

    FfsDssOperationData_t dssOperation = {
        .id = FFS_DSS_OPERATION_ID_REPORT,
        .name = "TEST",
        .path = "/test",
        {
            ffsDssClientHandleStatusCode,
            ffsDssClientHandleHeader,
            ffsDssClientHandleBody,
            ffsDssClientHandleRedirect,
            ffsDssClientBeforeRetry
        }
    };

    FfsStream_t requestBodyStream = FFS_STRING_INPUT_STREAM(REQUEST_BODY);
    FfsStream_t responseBodyStream1 = FFS_STRING_INPUT_STREAM(RESPONSE_BODY);
    FfsStream_t responseBodyStream2 = FFS_STRING_INPUT_STREAM(RESPONSE_BODY);
    FfsStream_t signatureHeaderKeyStream1 = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_KEY);
    FfsStream_t signatureHeaderKeyStream2 = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_KEY);
    FfsStream_t signatureHeaderValueStream1 = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_VALUE);
    FfsStream_t signatureHeaderValueStream2 = FFS_STRING_INPUT_STREAM(SIGNATURE_HEADER_VALUE);

    int operationData = 6;

    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), RequestBodyMatches(REQUEST_BODY), ResponseOperationDataMatches(&operationData)))
            .WillOnce(DoAll(ExecuteHandleStatusCodeCallback(HTTP_OK),
                    ExecuteHandleHeaderCallback(&signatureHeaderKeyStream1, &signatureHeaderValueStream1),
                    ExecuteHandleBodyCallback(&responseBodyStream1),
                    ExecuteBeforeRetryCallback(),
                    ExecuteHandleStatusCodeCallback(HTTP_OK),
                    ExecuteHandleHeaderCallback(&signatureHeaderKeyStream2, &signatureHeaderValueStream2),
                    ExecuteHandleBodyCallback(&responseBodyStream2),
                    Return(FFS_SUCCESS)));

    EXPECT_COMPAT_CALL(ffsVerifyCloudSignature(getUserContext(), PointeeStreamEqString(RESPONSE_BODY), PointeeStreamEqString(SIGNATURE), _))
            .WillOnce(DoAll(SetArgPointee<3>(false), Return(FFS_SUCCESS)))
            .WillOnce(DoAll(SetArgPointee<3>(true), Return(FFS_SUCCESS)));

    ASSERT_SUCCESS(ffsDssClientExecute(getDssClientContext(), &dssOperation, &requestBodyStream,
            &operationData));
}
