#include "constants/test_constants.h"
#include "helpers/test_utilities.h"
#include "ffs/dss/ffs_dss_operation_start_pin_based_setup.h"

#define DSS_SIGNATURE_HEADER_KEY        "x-amzn-dss-signature"
#define DSS_SIGNATURE_HEADER_VALUE      "SIGNATURE"

/** @brief "Salted PIN matches" matcher.
 */
MATCHER_P(SaltedPinMatches, sourceSaltedPin, "Salted PIN matches")
{
    return arraysAreEqual(FFS_STREAM_NEXT_READ(*arg),
            FFS_STREAM_DATA_SIZE(*arg),
            (uint8_t *) sourceSaltedPin,
            strlen(sourceSaltedPin));
}

/** @brief "Request has specified body" matcher.
 */
MATCHER_P(RequestBodyMatches, sourceRequestBody, "Request body matches")
{
    return arraysAreEqual(FFS_STREAM_NEXT_READ(arg->bodyStream),
            FFS_STREAM_DATA_SIZE(arg->bodyStream),
            (uint8_t *) sourceRequestBody,
            strlen(sourceRequestBody));
}

class DssStartPinBasedSetupTests: public TestContextFixture {
public:

    /** @brief Test the request callbacks with mock data.
     */
    static void handleStartPinBasedSetupResponse(struct FfsUserContext_s *userContext,
            FfsHttpRequest_t *request, void *callbackDataPointer) {
        (void) userContext;

        FfsHttpCallbacks_t callbacks = request->callbacks;

        const char *MOCK_RESPONSE_BODY_STRING = "{"
                "\"nonce\":\"" TEST_NONCE "\","
                "\"canProceed\":true"
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

/** @brief Test a 'start PIN-based setup' DSS operation.
 */
TEST_F(DssStartPinBasedSetupTests, StartPinBasedSetup)
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

    // Set the session ID.
    ASSERT_SUCCESS(ffsDssClientSetSessionId(&dssClientContext, TEST_SESSION_ID));

    // Create the salt stream.
    FfsStream_t saltStream = FFS_STRING_INPUT_STREAM(TEST_SALT);

    // Can proceed?
    bool canProceed = false;

    // Expected request body.
    const char *EXPECTED_REQUEST_BODY = "{"
            "\"nonce\":\"" TEST_NONCE "\","
            "\"sessionId\":\"" TEST_SESSION_ID "\","
            "\"deviceDetails\":"
            "{"
                "\"manufacturer\":\"" TEST_MANUFACTURER_NAME "\","
                "\"deviceModel\":\"" TEST_DEVICE_MODEL_NUMBER "\","
                "\"deviceSerial\":\"" TEST_DEVICE_SERIAL_NUMBER "\","
                "\"deviceName\":\"" TEST_BLE_DEVICE_NAME "\","
                "\"firmwareVersion\":\"" TEST_DEVICE_FIRMWARE_REVISION "\","
                "\"hardwareVersion\":\"" TEST_DEVICE_HARDWARE_REVISION "\""
            "},"
            "\"hashedPin\":\"" TEST_HASHED_DEVICE_PIN "\""
            "}";

    // Mock device information.
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_MANUFACTURER_NAME), _))
            .WillOnce(DoAll(WriteStringToMapValueArgPointee<2>(TEST_MANUFACTURER_NAME), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_BLE_DEVICE_NAME), _))
            .WillOnce(DoAll(WriteStringToMapValueArgPointee<2>(TEST_BLE_DEVICE_NAME), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_MODEL_NUMBER), _))
            .WillOnce(DoAll(WriteStringToMapValueArgPointee<2>(TEST_DEVICE_MODEL_NUMBER), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_SERIAL_NUMBER), _))
            .WillOnce(DoAll(WriteStringToMapValueArgPointee<2>(TEST_DEVICE_SERIAL_NUMBER), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_PRODUCT_INDEX), _))
            .WillOnce(Return(FFS_NOT_IMPLEMENTED));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_SOFTWARE_VERSION_INDEX), _))
            .WillOnce(Return(FFS_NOT_IMPLEMENTED));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_FIRMWARE_VERSION), _))
            .WillOnce(DoAll(WriteStringToMapValueArgPointee<2>(TEST_DEVICE_FIRMWARE_REVISION), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_HARDWARE_VERSION), _))
            .WillOnce(DoAll(WriteStringToMapValueArgPointee<2>(TEST_DEVICE_HARDWARE_REVISION), Return(FFS_SUCCESS)));

    // Mock 'ffsRandomBytes'.
    EXPECT_COMPAT_CALL(ffsRandomBytes(_, PointeeSpaceIs(strlen(TEST_RANDOM))))
            .WillRepeatedly(DoAll(WriteStringToArgPointee<1>(TEST_RANDOM), Return(FFS_SUCCESS)));

    // Mock 'ffsSha256'. Return "not implemented" to trigger the test SHA256 implementation.
    EXPECT_COMPAT_CALL(ffsSha256(_, SaltedPinMatches(TEST_SALTED_DEVICE_PIN), _))
            .WillOnce(Return(FFS_NOT_IMPLEMENTED));

    // Mock 'ffsHttpExecute'.
    EXPECT_COMPAT_CALL(ffsHttpExecute(_, RequestBodyMatches(EXPECTED_REQUEST_BODY), _))
            .WillOnce(DoAll(Invoke(handleStartPinBasedSetupResponse), Return(FFS_SUCCESS)));

    // Mock 'ffsGetConfigurationValue'.
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_, FFS_CONFIGURATION_ENTRY_KEY_PIN, _))
            .WillOnce(DoAll(WriteStringToMapValueArgPointee<2>(TEST_DEVICE_PIN), Return(FFS_SUCCESS)));

    // Execute the operation.
    ASSERT_SUCCESS(ffsDssStartPinBasedSetup(&dssClientContext, &canProceed, saltStream));

    // Assert that we can proceed.
    ASSERT_EQ(canProceed, true);
}
