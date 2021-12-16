/** @file ffs_dss_operation_report_tests.cpp
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
#include "constants/test_constants.h"
#include "helpers/test_utilities.h"
#include "ffs/dss/ffs_dss_operation_report.h"

#define DSS_SIGNATURE_HEADER_KEY        "x-amzn-dss-signature"
#define DSS_SIGNATURE_HEADER_VALUE      "SIGNATURE"

// Static functions.
extern "C" {
static FFS_RESULT getConnectionAttemptsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiConnectionAttempt_t *dssWifiConnectionAttempt, void *callbackDataPointer);
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

class DssReportTests: public TestContextFixture {
public:

    /** @brief Test the request callbacks with mock data.
     */
    static void handleReportResponse(struct FfsUserContext_s *userContext, FfsHttpRequest_t *request,
            void *callbackDataPointer) {
        (void) userContext;

        FfsHttpCallbacks_t callbacks = request->callbacks;

        const char *MOCK_RESPONSE_BODY_STRING = "{"
                "\"nonce\":\"" TEST_NONCE "\","
                "\"canProceed\":true,"
                "\"nextProvisioningState\":\"GET_WIFI_LIST\""
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

    /** @brief Ffs error details constructor.
     */
    static FfsDssErrorDetails_t ffsErrorDetailsConstructor(const char *operation, const char *cause, const char *code, const char *details) {
        FfsDssErrorDetails_t errorDetails;
        memset(&errorDetails, 0, sizeof(errorDetails));
        errorDetails.operation = operation;
        errorDetails.cause = cause;
        errorDetails.code = code;
        errorDetails.details = details;
        return errorDetails;
    }

    /** @brief Ffs Wi-Fi connection details constructor.
     */
    static FfsDssWifiConnectionDetails_t ffsWifiConnectionDetailsConstructor(FfsStream_t ssidStream,
            FFS_DSS_WIFI_SECURITY_PROTOCOL securityProtocol, FFS_DSS_WIFI_CONNECTION_STATE state,
            FfsDssErrorDetails_t *errorDetails) {
        FfsDssWifiConnectionDetails_t connectionDetails;
        connectionDetails.ssidStream = ssidStream;
        connectionDetails.securityProtocol = securityProtocol;
        connectionDetails.state = state;
        connectionDetails.hasErrorDetails = true,
        connectionDetails.errorDetails = *errorDetails;
        return connectionDetails;
    }
};

/** @brief Test generating a DSS report 'compute configuration' request.
 */
TEST_F(DssReportTests, ReportComputeConfiguration)
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
    ASSERT_SUCCESS(ffsDssClientSetSessionId(&dssClientContext, TEST_SESSION_ID));

    dssClientContext.sequenceNumber = 122;

    // Can proceed?
    bool canProceed = false;

    // Provisionee state.
    FFS_DSS_WIFI_PROVISIONEE_STATE PROVISIONEE_STATE = FFS_DSS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION;
    FFS_DSS_WIFI_PROVISIONEE_STATE EXPECTED_NEXT_PROVISIONEE_STATE = FFS_DSS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST;
    FFS_DSS_WIFI_PROVISIONEE_STATE nextProvisioneeState;

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
            "\"sequenceNumber\":123,"
            "\"currentProvisioningState\":\"COMPUTE_CONFIGURATION\","
            "\"registrationState\":\"FAILED\","
            "\"stateTransitionResult\":\"SUCCESS\","
            "\"wifiNetworkInfoList\":"
            "[{"
                    "\"ssid\":\"\\\"ssid1\\\"\","
                    "\"securityProtocol\":\"WPA_PSK\","
                    "\"wifiConnectionState\":\"DISCONNECTED\","
                    "\"errorDetails\":{\"operation\":\"report\",\"cause\":\"testing\",\"code\":\"1.2.3\"}"
            "},{"
                    "\"ssid\":\"\\\"ssid2\\\"\","
                    "\"securityProtocol\":\"WEP\","
                    "\"wifiConnectionState\":\"ASSOCIATED\","
                    "\"errorDetails\":{\"operation\":\"scan\",\"cause\":\"unknown\",\"details\":\"no code\"}"
            "}]}";

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

    // Mock 'ffsHttpExecute'.
    EXPECT_COMPAT_CALL(ffsHttpExecute(_, RequestBodyMatches(EXPECTED_REQUEST_BODY), _))
            .WillOnce(DoAll(Invoke(handleReportResponse), Return(FFS_SUCCESS)));

    // Execute the operation.
    ASSERT_SUCCESS(ffsDssReport(&dssClientContext,
            PROVISIONEE_STATE,
            FFS_DSS_REPORT_RESULT_SUCCESS,
            FFS_DSS_REGISTRATION_STATE_FAILED,
            &canProceed,
            &nextProvisioneeState,
            getConnectionAttemptsCallback,
            NULL));

    // Check that we got the expected next provisionee state.
    ASSERT_EQ(nextProvisioneeState, EXPECTED_NEXT_PROVISIONEE_STATE);

    // Check that we can proceed.
    ASSERT_EQ(canProceed, true);
}

extern "C" {

/** @brief Callback to get the Wi-Fi connection attempts to report.
 *
 * @param userContext User context
 * @param callbackDataPointer Pointer to "connection attempts" data
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT getConnectionAttemptsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiConnectionAttempt_t *dssWifiConnectionAttempt, void *callbackDataPointer)
{
    (void) userContext;
    (void) dssWifiConnectionAttempt;

    // Wi-Fi connection details 1.
    FfsDssWifiConnectionDetails_t connectionDetails;
    memset(&connectionDetails, 0, sizeof(connectionDetails));
    connectionDetails.ssidStream = FFS_STRING_INPUT_STREAM("ssid1");
    connectionDetails.securityProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
    connectionDetails.state = FFS_DSS_WIFI_CONNECTION_STATE_DISCONNECTED;
    connectionDetails.hasErrorDetails = true;
    connectionDetails.errorDetails.operation = "report";
    connectionDetails.errorDetails.cause = "testing";
    connectionDetails.errorDetails.code = "1.2.3";
    connectionDetails.errorDetails.details = NULL;
    FFS_CHECK_RESULT(ffsDssReportAddConnectionAttempt(callbackDataPointer, &connectionDetails));

    connectionDetails.ssidStream = FFS_STRING_INPUT_STREAM("ssid2");
    connectionDetails.securityProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_WEP;
    connectionDetails.state = FFS_DSS_WIFI_CONNECTION_STATE_ASSOCIATED;
    connectionDetails.hasErrorDetails = true;
    connectionDetails.errorDetails.operation = "scan";
    connectionDetails.errorDetails.cause = "unknown";
    connectionDetails.errorDetails.code = NULL;
    connectionDetails.errorDetails.details = "no code";
    FFS_CHECK_RESULT(ffsDssReportAddConnectionAttempt(callbackDataPointer, &connectionDetails));

    return FFS_SUCCESS;
}

}
