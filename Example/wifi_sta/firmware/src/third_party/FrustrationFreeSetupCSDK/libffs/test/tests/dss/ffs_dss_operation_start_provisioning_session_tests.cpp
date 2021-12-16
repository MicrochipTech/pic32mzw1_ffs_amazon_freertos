/** @file ffs_dss_operation_start_provisioning_session_tests.cpp
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
#include "ffs/common/ffs_logging.h"
#include "constants/test_constants.h"
#include "helpers/test_utilities.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/dss/ffs_dss_operation_start_provisioning_session.h"

#define DSS_SIGNATURE_HEADER_KEY        "x-amzn-dss-signature"
#define DSS_SIGNATURE_HEADER_VALUE      "SIGNATURE"

/** @brief "Request has specified body" matcher.
 */
MATCHER_P(RequestBodyMatches, sourceRequestBody, "Request body matches")
{
    return arraysAreEqual(FFS_STREAM_NEXT_READ(arg->bodyStream),
            FFS_STREAM_DATA_SIZE(arg->bodyStream),
            (uint8_t *) sourceRequestBody,
            strlen(sourceRequestBody));
}

/** @brief "Map value matches" matcher.
 */
MATCHER_P(MapValueMatches, sourceMapValue, "Configuration entry matches")
{
    return arg->type == sourceMapValue.type
            && ffsStreamMatchesStream(&arg->stringStream, (FfsStream_t *)&sourceMapValue.stringStream);
}

/** @brief "Response has specified session ID" matcher.
 */
MATCHER_P(SessionIdMatches, sourceSessionId, "Session ID matches")
{
    return (strncmp(arg, sourceSessionId, strlen(sourceSessionId)) == 0);
}

class DssStartProvisioningSessionTests: public TestContextFixture {
public:

    /** @brief Test the request callbacks with mock data.
     */
    static void handleStartProvisioningSessionResponse(struct FfsUserContext_s *userContext,
            FfsHttpRequest_t *request, void *callbackDataPointer) {
        (void) userContext;

        FfsHttpCallbacks_t callbacks = request->callbacks;

        const char *MOCK_RESPONSE_BODY_STRING = "{"
                "\"nonce\":\"" TEST_NONCE "\","
                "\"sessionId\":\"" TEST_SESSION_ID "\","
                "\"canProceed\":true,"
                "\"salt\":\"" TEST_SALT "\""
                "}";
        uint8_t MOCK_RESPONSE_BODY_BUFFER[strlen(MOCK_RESPONSE_BODY_STRING)];
        memcpy(MOCK_RESPONSE_BODY_BUFFER, MOCK_RESPONSE_BODY_STRING, strlen(MOCK_RESPONSE_BODY_STRING));

        // Mock signature header.
        FfsStream_t MOCK_SIGNATURE_HEADER_KEY_STREAM = FFS_STRING_INPUT_STREAM(DSS_SIGNATURE_HEADER_KEY);
        FfsStream_t MOCK_SIGNATURE_HEADER_VALUE_STREAM = FFS_STRING_INPUT_STREAM(DSS_SIGNATURE_HEADER_VALUE);

        // Mock response body.
        FfsStream_t MOCK_RESPONSE_BODY_STREAM = ffsCreateInputStream(MOCK_RESPONSE_BODY_BUFFER, strlen(MOCK_RESPONSE_BODY_STRING));

        // Handle status code.
        ASSERT_SUCCESS(callbacks.handleStatusCode(200, callbackDataPointer));

        // Handle (signature) header.
        ASSERT_SUCCESS(callbacks.handleHeader(&MOCK_SIGNATURE_HEADER_KEY_STREAM, &MOCK_SIGNATURE_HEADER_VALUE_STREAM, callbackDataPointer));

        // Handle response body.
        ASSERT_SUCCESS(callbacks.handleBody(&MOCK_RESPONSE_BODY_STREAM, callbackDataPointer));
    }

};

/** @brief Test a 'start provisioning session' DSS operation.
 */
TEST_F(DssStartProvisioningSessionTests, StartProvisioningSession)
{
    // DSS client context.
    FFS_TEMPORARY_OUTPUT_STREAM(hostStream, 256);
    FFS_TEMPORARY_OUTPUT_STREAM(sessionIdStream, 256);
    FFS_TEMPORARY_OUTPUT_STREAM(nonceStream, 16);
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, 1024);
    FfsDssClientContext_t dssClientContext;
    EXPECT_COMPAT_CALL(ffsDssClientGetBuffers(_, _, _, _, _)).WillOnce(DoAll(
            SetArgPointee<1>(hostStream),
            SetArgPointee<2>(sessionIdStream),
            SetArgPointee<3>(nonceStream),
            SetArgPointee<4>(bodyStream),
            Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(getUserContext(), FFS_CONFIGURATION_ENTRY_KEY_DSS_HOST, _))
            .WillOnce(Return(FFS_NOT_IMPLEMENTED));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(getUserContext(), FFS_CONFIGURATION_ENTRY_KEY_DSS_PORT, _))
            .WillOnce(Return(FFS_NOT_IMPLEMENTED));
    EXPECT_COMPAT_CALL(ffsVerifyCloudSignature(_, _, _, _))
            .WillOnce(DoAll(SetArgPointee<3>(true),
                    Return(FFS_SUCCESS)));
    ASSERT_SUCCESS(ffsDssClientInit(getUserContext(), &dssClientContext));

    // Create the salt stream.
    FFS_TEMPORARY_OUTPUT_STREAM(saltStream, FFS_SALT_SIZE);

    // Can proceed?
    bool canProceed = false;

    // Expected request body.
    const char *EXPECTED_REQUEST_BODY = "{\"nonce\":\"" TEST_NONCE "\"}";

    // Mock 'ffsRandomBytes'.
    EXPECT_COMPAT_CALL(ffsRandomBytes(_, PointeeSpaceIs(strlen(TEST_RANDOM))))
            .WillRepeatedly(DoAll(WriteStringToArgPointee<1>(TEST_RANDOM), Return(FFS_SUCCESS)));

    // Mock 'ffsHttpExecute'.
    EXPECT_COMPAT_CALL(ffsHttpExecute(_, RequestBodyMatches(EXPECTED_REQUEST_BODY), _))
            .WillOnce(DoAll(Invoke(handleStartProvisioningSessionResponse), Return(FFS_SUCCESS)));

    // Execute the operation.
    ASSERT_SUCCESS(ffsDssStartProvisioningSession(&dssClientContext, &canProceed, &saltStream));

    // Assert the session ID was stored.
    const char *sessionId;
    ASSERT_SUCCESS(ffsDssClientGetSessionId(&dssClientContext, &sessionId));
    ASSERT_STREQ(sessionId, TEST_SESSION_ID);

    // Assert that we can proceed.
    ASSERT_EQ(canProceed, true);

    // Assert that we got the salt back.
    ASSERT_STREAM_EQ_STRING(saltStream, TEST_SALT);
}
