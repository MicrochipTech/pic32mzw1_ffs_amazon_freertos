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
#include "ffs/conversion/ffs_convert_wifi_credentials.h"
#include "constants/test_constants.h"
#include "helpers/test_utilities.h"
#include "ffs/dss/ffs_dss_operation_get_wifi_credentials.h"

#define DSS_SIGNATURE_HEADER_KEY        "x-amzn-dss-signature"
#define DSS_SIGNATURE_HEADER_VALUE      "SIGNATURE"

#define TEST_ASCII_WEP_KEY "11111"
#define TEST_HEX_WEP_KEY   "3131313131"

extern "C" {

// Static functions.
static FFS_RESULT saveWifiCredentialsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiCredentials_t *wifiCredentials);

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

class DssGetWifiCredentialsTests: public TestContextFixture {
public:

    /** @brief Test the request callbacks with mock data.
     */
    static void handleGetWifiCredentialsResponse(struct FfsUserContext_s *userContext,
            FfsHttpRequest_t *request, void *callbackDataPointer) {

        (void) userContext;

        FfsHttpCallbacks_t callbacks = request->callbacks;

        const char *MOCK_RESPONSE_BODY_STRING = "{"
                "\"nonce\":\"" TEST_NONCE "\","
                "\"canProceed\":true,"
                "\"sequenceNumber\":1,"
                "\"allCredentialsReturned\":true,"
                "\"wifiCredentialsList\":["
                "{"
                        "\"frequency\":0,"
                        "\"keyIndex\":0,"
                        "\"priority\":1,"
                        "\"securityProtocol\":\"OPEN\","
                        "\"ssid\":\"\\\"TEST_OPEN\\\"\""
                "},{"
                        "\"frequency\":0,"
                        "\"key\":\"\\\"TEST_WPA_KEY\\\"\","
                        "\"keyIndex\":0,"
                        "\"priority\":1,"
                        "\"securityProtocol\":\"WPA_PSK\","
                        "\"ssid\":\"\\\"TEST_WPA\\\"\""
                "},{"
                        "\"frequency\":0,"
                        "\"key\":\"\\\"" TEST_ASCII_WEP_KEY "\\\"\","
                        "\"keyIndex\":0,"
                        "\"priority\":1,"
                        "\"securityProtocol\":\"WEP\","
                        "\"ssid\":\"\\\"TEST_ASCII_WEP\\\"\""
                "},{"
                        "\"frequency\":0,"
                        "\"key\":\"" TEST_HEX_WEP_KEY "\","
                        "\"keyIndex\":0,"
                        "\"priority\":1,"
                        "\"securityProtocol\":\"WEP\","
                        "\"ssid\":\"\\\"TEST_HEX_WEP\\\"\""
                "}"
                "]}";
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

    /** @brief Ffs Wi-Fi configuration constructor.
     */
    static FfsWifiConfiguration_t ffsWifiConfigurationConstructor(const char *ssid, const char *key,
            FFS_WIFI_SECURITY_PROTOCOL securityProtocol) {
        FfsWifiConfiguration_t configuration;
        configuration.ssidStream = FFS_STRING_INPUT_STREAM(ssid);
        configuration.securityProtocol = securityProtocol;
        if (key) {
            configuration.keyStream = FFS_STRING_INPUT_STREAM(key);
        } else {
            ffsSetStreamToNull(&configuration.keyStream);
        }
        configuration.wepIndex = 0;
        return configuration;
    }

};

/** @brief Test a "get Wi-Fi credentials" DSS operation.
 */
TEST_F(DssGetWifiCredentialsTests, GetWifiCredentials)
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

    // All credentials returned?
    bool allCredentialsReturned = false;

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
            "\"sequenceNumber\":1"
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

    // Mock 'ffsHttpExecute'.
    EXPECT_COMPAT_CALL(ffsHttpExecute(_, RequestBodyMatches(EXPECTED_REQUEST_BODY), _))
            .WillOnce(DoAll(Invoke(handleGetWifiCredentialsResponse), Return(FFS_SUCCESS)));

    // Mock 'ffsAddWifiConfiguration'.
    FfsWifiConfiguration_t EXPECTED_WIFI_CONFIGURATION1 = ffsWifiConfigurationConstructor("TEST_OPEN", NULL,
            FFS_WIFI_SECURITY_PROTOCOL_NONE);
    FfsWifiConfiguration_t EXPECTED_WIFI_CONFIGURATION2 = ffsWifiConfigurationConstructor("TEST_WPA", "TEST_WPA_KEY",
            FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK);
    FfsWifiConfiguration_t EXPECTED_WIFI_CONFIGURATION3 = ffsWifiConfigurationConstructor("TEST_ASCII_WEP", TEST_ASCII_WEP_KEY,
            FFS_WIFI_SECURITY_PROTOCOL_WEP);
    FfsWifiConfiguration_t EXPECTED_WIFI_CONFIGURATION4 = ffsWifiConfigurationConstructor("TEST_HEX_WEP", TEST_ASCII_WEP_KEY, //!< Expect hex key as ASCII
            FFS_WIFI_SECURITY_PROTOCOL_WEP);
    EXPECT_COMPAT_CALL(ffsAddWifiConfiguration(_, WifiConfigurationMatches(EXPECTED_WIFI_CONFIGURATION1))).WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsAddWifiConfiguration(_, WifiConfigurationMatches(EXPECTED_WIFI_CONFIGURATION2))).WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsAddWifiConfiguration(_, WifiConfigurationMatches(EXPECTED_WIFI_CONFIGURATION3))).WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsAddWifiConfiguration(_, WifiConfigurationMatches(EXPECTED_WIFI_CONFIGURATION4))).WillOnce(Return(FFS_SUCCESS));

    // Execute the operation.
    ASSERT_SUCCESS(ffsDssGetWifiCredentials(&dssClientContext,
            &canProceed,
            1,
            saveWifiCredentialsCallback,
            &allCredentialsReturned));

    // Assert that we can proceed.
    ASSERT_EQ(canProceed, true);

    // Assert that we got all credentials.
    ASSERT_EQ(allCredentialsReturned, true);
}

extern "C" {

/** @brief Callback to save Wi-Fi credentials.
 *
 * @param userContext User context
 * @param wifiCredentials Wi-Fi credentials to save
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT saveWifiCredentialsCallback(struct FfsUserContext_s *userContext,
        FfsDssWifiCredentials_t *wifiCredentials)
{
    FfsWifiConfiguration_t wifiConfiguration;
    FFS_CHECK_RESULT(ffsConvertDssWifiCredentialsToApi(wifiCredentials, &wifiConfiguration));
    FFS_CHECK_RESULT(ffsAddWifiConfiguration(userContext, &wifiConfiguration));

    return FFS_SUCCESS;
}

}
