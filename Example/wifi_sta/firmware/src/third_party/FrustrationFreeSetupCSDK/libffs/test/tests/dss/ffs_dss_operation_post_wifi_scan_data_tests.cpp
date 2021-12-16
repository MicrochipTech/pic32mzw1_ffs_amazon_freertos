/** @file ffs_dss_operation_get_wifi_credentials_tests.cpp
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
#include "ffs/dss/ffs_dss_operation_post_wifi_scan_data.h"

#define DSS_SIGNATURE_HEADER_KEY        "x-amzn-dss-signature"
#define DSS_SIGNATURE_HEADER_VALUE      "SIGNATURE"

extern "C" {

// Static functions.
static FFS_RESULT getWifiScanResultsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiScanResult_t *dssWifiScanResult, void *callbackDataPointer);

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

/** @brief Wi-Fi configuration matcher.
 */
MATCHER_P(WifiConfigurationMatches, sourceWifiConfiguration, "Wi-Fi configuration matches")
{
    if (arg->securityProtocol != sourceWifiConfiguration.securityProtocol) {
        return false;
    }
    if (!ffsStreamMatchesStream(&arg->ssidStream, (FfsStream_t *)&sourceWifiConfiguration.ssidStream)) {
        return false;
    }
    if (arg->securityProtocol != FFS_WIFI_SECURITY_PROTOCOL_NONE
            && !ffsStreamMatchesStream(&arg->keyStream, (FfsStream_t *)&sourceWifiConfiguration.keyStream)) {
        return false;
    }
    if (arg->securityProtocol == FFS_WIFI_SECURITY_PROTOCOL_WEP
            && arg->wepIndex != sourceWifiConfiguration.wepIndex) {
        return false;
    }

    return true;
}

class DssPostWifiScanDataTests: public TestContextFixture {
public:

    /** @brief Test the request callbacks with mock data.
     */
    static void handlePostWifiScanDataResponse(struct FfsUserContext_s *userContext,
            FfsHttpRequest_t *request, void *callbackDataPointer) {

        (void) userContext;

        FfsHttpCallbacks_t callbacks = request->callbacks;

        const char *MOCK_RESPONSE_BODY_STRING = "{"
                "\"nonce\":\"" TEST_NONCE "\","
                "\"sessionId\":\"" TEST_SESSION_ID "\","
                "\"canProceed\":true,"
                "\"sequenceNumber\":1,"
                "\"totalCredentialsFound\":3,"
                "\"allCredentialsFound\":true"
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

/** @brief Test a "post Wi-Fi scan data" DSS operation.
 */
TEST_F(DssPostWifiScanDataTests, PostWifiScanData)
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

    // Can proceed?
    bool canProceed = false;

    // Total credentials found.
    uint32_t totalCredentialsFound = 0;

    // All credentials found?
    bool allCredentialsFound = false;

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
            "\"sequenceNumber\":1,"
            "\"wifiScanDataList\":["
                "{"
                    "\"ssid\":\"\\\"TEST_OPEN\\\"\","
                    "\"bssid\":\"00:00:00:00:00:00\","
                    "\"securityProtocol\":\"OPEN\","
                    "\"rssi\":1,"
                    "\"frequency\":1"
                "},{"
                    "\"ssid\":\"\\\"TEST_WPA_PSK\\\"\","
                    "\"bssid\":\"11:11:11:11:11:11\","
                    "\"securityProtocol\":\"WPA_PSK\","
                    "\"rssi\":1,"
                    "\"frequency\":1"
                "},{"
                    "\"ssid\":\"\\\"TEST_WEP\\\"\","
                    "\"bssid\":\"22:22:22:22:22:22\","
                    "\"securityProtocol\":\"WEP\","
                    "\"rssi\":1,"
                    "\"frequency\":1"
                "}"
            "]}";

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
            .WillOnce(DoAll(Invoke(handlePostWifiScanDataResponse), Return(FFS_SUCCESS)));

    // Execute the operation.
    ASSERT_SUCCESS(ffsDssPostWifiScanData(&dssClientContext,
            &canProceed,
            1,
            getWifiScanResultsCallback,
            NULL,
            &totalCredentialsFound,
            &allCredentialsFound));

    // Assert that we can proceed.
    ASSERT_EQ(canProceed, true);

    // Assert that we recorded the expected number of credentials found.
    ASSERT_EQ(totalCredentialsFound, (uint32_t)3);

    // Assert that we found all credentials.
    ASSERT_EQ(allCredentialsFound, true);

}

/** @brief Callback to get Wi-Fi scan results.
 *
 * @param userContext User context
 * @param callbackDataPointer Pointer to "get Wi-Fi scan results" data
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT getWifiScanResultsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiScanResult_t *dssWifiScanResult, void *callbackDataPointer)
{
    (void) userContext;
    (void) dssWifiScanResult;

    FfsDssWifiScanResult_t scanResult;
    memset(&scanResult, 0, sizeof(scanResult));

    // Scan result 1.
    uint8_t BSSID1[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    scanResult.ssidStream = FFS_STRING_INPUT_STREAM("TEST_OPEN");
    scanResult.bssidStream = FFS_STATIC_INPUT_STREAM(BSSID1);
    scanResult.securityProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_OPEN;
    scanResult.frequencyBand = 1;
    scanResult.signalStrength = 1;
    FFS_CHECK_RESULT(ffsDssPostWifiScanDataAddScanResult(callbackDataPointer, &scanResult));

    // Scan result 2.
    uint8_t BSSID2[] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
    scanResult.ssidStream = FFS_STRING_INPUT_STREAM("TEST_WPA_PSK");
    scanResult.bssidStream = FFS_STATIC_INPUT_STREAM(BSSID2);
    scanResult.securityProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
    scanResult.frequencyBand = 1;
    scanResult.signalStrength = 1;
    FFS_CHECK_RESULT(ffsDssPostWifiScanDataAddScanResult(callbackDataPointer, &scanResult));

    // Scan result 3.
    uint8_t BSSID3[] = { 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 };
    scanResult.ssidStream = FFS_STRING_INPUT_STREAM("TEST_WEP");
    scanResult.bssidStream = FFS_STATIC_INPUT_STREAM(BSSID3);
    scanResult.securityProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_WEP;
    scanResult.frequencyBand = 1;
    scanResult.signalStrength = 1;
    FFS_CHECK_RESULT(ffsDssPostWifiScanDataAddScanResult(callbackDataPointer, &scanResult));

    return FFS_SUCCESS;
}
