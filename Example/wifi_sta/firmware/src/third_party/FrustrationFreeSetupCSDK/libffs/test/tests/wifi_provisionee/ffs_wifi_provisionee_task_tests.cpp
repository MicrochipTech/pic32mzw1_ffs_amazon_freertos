/** @file ffs_provisionee_tests.cpp
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

#include "constants/test_constants.h"
#include "helpers/test_utilities.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_task.h"

/** @brief "Public key matches" matcher.
 */
MATCHER_P(PublicKeyMatches, sourcePublicKey, "Salted PIN matches")
{
    return arraysAreEqual(FFS_STREAM_NEXT_READ(*arg),
            FFS_STREAM_DATA_SIZE(*arg),
            FFS_STREAM_NEXT_READ(sourcePublicKey),
            FFS_STREAM_DATA_SIZE(sourcePublicKey));
}

/** @brief "Salted PIN matches" matcher.
 */
MATCHER_P(SaltedPinMatches, sourceSaltedPin, "Salted PIN matches")
{
    return arraysAreEqual(FFS_STREAM_NEXT_READ(*arg),
            FFS_STREAM_DATA_SIZE(*arg),
            (uint8_t *) sourceSaltedPin,
            strlen(sourceSaltedPin));
}

/** @brief "Map value matches" matcher.
 */
MATCHER_P(MapValueMatches, sourceMapValue, "Configuration entry matches")
{
    return arg->type == sourceMapValue.type
            && ffsStreamMatchesStream(&arg->stringStream, (FfsStream_t *)&sourceMapValue.stringStream);
}

#define DSS_HOST_BUFFER_SIZE            (256)
#define DSS_SESSION_ID_BUFFER_SIZE      (1024)
#define DSS_NONCE_BUFFER_SIZE           (32)
#define DSS_BODY_BUFFER_SIZE            (2048)
#define DSS_SIGNATURE_HEADER_KEY        "x-amzn-dss-signature"
#define DSS_SIGNATURE_HEADER_VALUE      "SIGNATURE"
#define ENCODED_SSID_NONCE_SIZE         (12)

class TaskTests: public TestContextFixture {
public:

    /** @brief Ffs registration request constructor.
     */
    static FfsRegistrationRequest_t ffsRegistrationRequestConstructor(const char *token, int64_t expiration) {
        FfsRegistrationRequest_t registrationRequest;
        registrationRequest.tokenStream = FFS_STRING_INPUT_STREAM(token);
        registrationRequest.expiration = expiration;
        return registrationRequest;
    }

    /** @brief Get the Ffs locale map string value
     */
    static void ffsSetLocaleValue(struct FfsUserContext_s *userContext, const char *key,
            FfsMapValue_t *value) {
        (void) userContext;
        (void) key;
        ffsWriteStringToStream(TEST_LANGUAGE_LOCALE, &value->stringStream);
        value->type = FFS_MAP_VALUE_TYPE_STRING;
    }

    /** @brief Get the Ffs country code map string value
     */
    static void ffsSetCountryCodeValue(struct FfsUserContext_s *userContext, const char *key,
            FfsMapValue_t *value) {
        (void) userContext;
        (void) key;
        ffsWriteStringToStream(TEST_COUNTRY_CODE, &value->stringStream);
        value->type = FFS_MAP_VALUE_TYPE_STRING;
    }

    /** @brief Get the Ffs country of residence map string value
     */
    static void ffsSetCountryOfResidenceValue(struct FfsUserContext_s *userContext, const char *key,
            FfsMapValue_t *value) {
        (void) userContext;
        (void) key;
        ffsWriteStringToStream(TEST_COUNTRY_OF_RESIDENCE, &value->stringStream);
        value->type = FFS_MAP_VALUE_TYPE_STRING;
    }

    /** @brief Get the Ffs region map string value
     */
    static void ffsSetRegionValue(struct FfsUserContext_s *userContext, const char *key,
            FfsMapValue_t *value) {
        (void) userContext;
        (void) key;
        ffsWriteStringToStream(TEST_REGION, &value->stringStream);
        value->type = FFS_MAP_VALUE_TYPE_STRING;
    }

    /** @brief Get the Ffs realm map string value
     */
    static void ffsSetRealmValue(struct FfsUserContext_s *userContext, const char *key,
            FfsMapValue_t *value) {
        (void) userContext;
        (void) key;
        ffsWriteStringToStream(TEST_REALM, &value->stringStream);
        value->type = FFS_MAP_VALUE_TYPE_STRING;
    }

    /** @brief Get the Ffs marketplace map string value
     */
    static void ffsSetMarketplaceValue(struct FfsUserContext_s *userContext, const char *key,
            FfsMapValue_t *value) {
        (void) userContext;
        (void) key;
        ffsWriteStringToStream(TEST_MARKETPLACE, &value->stringStream);
        value->type = FFS_MAP_VALUE_TYPE_STRING;
    }

};

static void sendSignatureResponse(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    FfsStream_t MOCK_SIGNATURE_HEADER_KEY_STREAM = FFS_STRING_INPUT_STREAM(DSS_SIGNATURE_HEADER_KEY);
    FfsStream_t MOCK_SIGNATURE_HEADER_VALUE_STREAM = FFS_STRING_INPUT_STREAM(DSS_SIGNATURE_HEADER_VALUE);
    ASSERT_SUCCESS(callbacks.handleStatusCode(200, callbackDataPointer));
    ASSERT_SUCCESS(callbacks.handleHeader(&MOCK_SIGNATURE_HEADER_KEY_STREAM, &MOCK_SIGNATURE_HEADER_VALUE_STREAM, callbackDataPointer));
}

static void sendNonceOnlyBody(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":true}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

static void sendStartProvisioningSessionBody(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":true,\"sessionId\":\"" TEST_SESSION_ID "\",\"salt\":\"" TEST_SALT "\"}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

static void sendStartProvisioningSessionBodyCannotProceed(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":false,\"sessionId\":\"" TEST_SESSION_ID "\",\"salt\":\"" TEST_SALT "\"}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

static void sendStartProvisioningSessionReportBody(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":true,\"nextProvisioningState\":\"START_PIN_BASED_SETUP\"}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

static void sendStartPinBasedSetupReportBody(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":true,\"nextProvisioningState\":\"COMPUTE_CONFIGURATION\"}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

static void sendComputeConfigurationReportBody(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":true,\"nextProvisioningState\":\"POST_WIFI_SCAN_DATA\"}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

static void sendPostWifiScanDataReportBody(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":true,\"nextProvisioningState\":\"GET_WIFI_LIST\"}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

static void sendGetWifiInfoBody(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":true,\"sequenceNumber\":1,\"allCredentialsReturned\":true}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

static void sendGetWifiInfoReportBody(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":true,\"nextProvisioningState\":\"CONNECTING_TO_USER_NETWORK\"}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

static void sendConnectingToUserNetworkReportBody(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":true,\"nextProvisioningState\":\"CONNECTED_TO_USER_NETWORK\"}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

static void sendConnectedToUserNetworkReportBody(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer) {
    (void) userContext;
    FfsHttpCallbacks_t callbacks = request->callbacks;
    const char BODY_STRING[] = "{\"nonce\":\"" TEST_NONCE "\",\"canProceed\":true,\"nextProvisioningState\":\"DONE\"}";
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, sizeof(BODY_STRING));
    ffsWriteStringToStream(BODY_STRING, &bodyStream);
    callbacks.handleBody(&bodyStream, callbackDataPointer);
}

TEST_F(TaskTests, TaskSuccessWithEncodedSSID)
{
    // Provisionee state.
    FFS_WIFI_PROVISIONEE_STATE state;
    EXPECT_COMPAT_CALL(ffsGetWifiProvisioneeState(getUserContext(), _))
            .Times(10) //!< Expect to handle 10 states.
            .WillRepeatedly(DoAll(SetArgPointee<1>(ByRef(state)), Return(FFS_SUCCESS)));

    // Mock 'ffsWifiProvisioneeCanProceed'.
    EXPECT_COMPAT_CALL(ffsWifiProvisioneeCanProceed(getUserContext(), _))
            .Times(10) //!< Expect to handle 10 states.
            .WillRepeatedly(DoAll(SetArgPointee<1>(true), Return(FFS_SUCCESS)));

    // Mock 'ffsDssClientGetBuffers'.
    FFS_TEMPORARY_OUTPUT_STREAM(hostStream, DSS_HOST_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(sessionIdStream, DSS_SESSION_ID_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(nonceStream, DSS_NONCE_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, DSS_BODY_BUFFER_SIZE);
    EXPECT_COMPAT_CALL(ffsDssClientGetBuffers(getUserContext(), _, _, _, _)).WillOnce(DoAll(
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
            .WillRepeatedly(DoAll(SetArgPointee<3>(true),
                    Return(FFS_SUCCESS)));
    
    // Mock the encoded SSID/passphrase related calls.
    FFS_LITERAL_INPUT_STREAM(randomNonceForSSIDStream, TEST_12_BYTE_NONCE);
    EXPECT_COMPAT_CALL(ffsRandomBytes(getUserContext(), PointeeSpaceIsSizeOf(randomNonceForSSIDStream)))
            .WillOnce(DoAll(WriteStreamToArgPointee<1>(randomNonceForSSIDStream), Return(FFS_SUCCESS)));

    // Mock public key.
    FFS_LITERAL_INPUT_STREAM(testPublicKeyStream, TEST_PUBLIC_KEY_DER_BYTES);
    FFS_LITERAL_INPUT_STREAM(testHashedPublicKeyStream, TEST_HASH_BYTES);
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(getUserContext(),
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_DEVICE_EC_PUBLIC_KEY_DER), _))
            .WillOnce(DoAll(WriteByteStreamToMapValueArgPointee<2>(testPublicKeyStream), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsSha256(_, PublicKeyMatches(testPublicKeyStream), _))
            .WillOnce(DoAll(WriteStreamToArgPointee<2>(testHashedPublicKeyStream), Return(FFS_SUCCESS)));

    // Mock product index.
    FFS_LITERAL_INPUT_STREAM(productIndexStream, { 0x11, 0x22, 0x33, 0x44});
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(getUserContext(),
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_PRODUCT_INDEX), _))
            .Times(12)
            .WillOnce(DoAll(WriteStringStreamToMapValueArgPointee<2>(productIndexStream), Return(FFS_SUCCESS)))
            .WillRepeatedly(Return(FFS_NOT_IMPLEMENTED));

    // Mock cloud public key.
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(getUserContext(),
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_CLOUD_EC_PUBLIC_KEY_DER), _))
            .WillOnce(DoAll(WriteByteStreamToMapValueArgPointee<2>(testPublicKeyStream), Return(FFS_SUCCESS)));

    // Mock ECDH shared secret.
    EXPECT_COMPAT_CALL(ffsComputeECDHKey(getUserContext(), PointeeStreamEq(testPublicKeyStream), PointeeSpaceIsSizeOf(testHashedPublicKeyStream)))
            .WillOnce(DoAll(WriteStreamToArgPointee<2>(testHashedPublicKeyStream), Return(FFS_SUCCESS)));

    // Mock HMAC calculation.
    EXPECT_COMPAT_CALL(ffsComputeHMACSHA256(getUserContext(), PointeeStreamEq(testHashedPublicKeyStream),
            PointeeStreamEq(randomNonceForSSIDStream), PointeeSpaceIsSizeOf(testHashedPublicKeyStream)))
                    .WillOnce(DoAll(WriteStreamToArgPointee<3>(testHashedPublicKeyStream), Return(FFS_SUCCESS)));

    // Mock 'ffsHttpExecute'.
    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), _, _))
            // 'START_PROVISIONING'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendStartProvisioningSessionBody),
                    Return(FFS_SUCCESS)))
            // Report 'START_PROVISIONING' and set next provisionee state to 'START_PIN_BASED_SETUP'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendStartProvisioningSessionReportBody),
                    Return(FFS_SUCCESS)))
            // 'START_PIN_BASED_SETUP'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendNonceOnlyBody),
                    Return(FFS_SUCCESS)))
            // Report 'START_PIN_BASED_SETUP' and set next provisionee state to 'COMPUTE_CONFIGURATION'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendStartPinBasedSetupReportBody),
                    Return(FFS_SUCCESS)))
            // 'COMPUTE_CONFIGURATION'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendNonceOnlyBody),
                    Return(FFS_SUCCESS)))
            // Report 'COMPUTE_CONFIGURATION' and set next provisionee state to 'POST_WIFI_SCAN_DATA'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendComputeConfigurationReportBody),
                    Return(FFS_SUCCESS)))
            // 'POST_WIFI_SCAN_DATA'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendNonceOnlyBody),
                    Return(FFS_SUCCESS)))
            // Report 'POST_WIFI_SCAN_DATA' and set next provisionee state to 'GET_WIFI_LIST'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendPostWifiScanDataReportBody),
                    Return(FFS_SUCCESS)))
            // 'GET_WIFI_LIST'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendGetWifiInfoBody),
                    Return(FFS_SUCCESS)))
            // Report 'GET_WIFI_LIST' and set next provisionee state to 'CONNECTING_TO_USER_NETWORK'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendGetWifiInfoReportBody),
                    Return(FFS_SUCCESS)))
            // Report 'CONNECT_TO_USER_NETWORK' and set next provisionee state to 'CONNECTED_TO_USER_NETWORK'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendConnectingToUserNetworkReportBody),
                    Return(FFS_SUCCESS)))
            // Report 'CONNECTED_TO_USER_NETWORK' and set next provisionee state to 'DONE'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendConnectedToUserNetworkReportBody),
                    Return(FFS_SUCCESS)));

    // Mock 'ffsGetRandomBytes'.
    EXPECT_COMPAT_CALL(ffsRandomBytes(_, PointeeSpaceIs(strlen(TEST_RANDOM))))
            .WillRepeatedly(DoAll(WriteStringToArgPointee<1>(TEST_RANDOM), Return(FFS_SUCCESS)));

    // Mock device information.
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_MANUFACTURER_NAME), _))
            .Times(11) //!< Seven Reports plus 'START_PROVISIONING_SESSION', 'START_PIN_BASED_SETUP', 'COMPUTE_CONFIGURATION', and 'POST_WIFI_SCAN_DATA'.
            .WillRepeatedly(DoAll(WriteStringToMapValueArgPointee<2>(TEST_MANUFACTURER_NAME), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_BLE_DEVICE_NAME), _))
            .Times(11)
            .WillRepeatedly(DoAll(WriteStringToMapValueArgPointee<2>(TEST_BLE_DEVICE_NAME), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_MODEL_NUMBER), _))
            .Times(11)
            .WillRepeatedly(DoAll(WriteStringToMapValueArgPointee<2>(TEST_DEVICE_MODEL_NUMBER), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_SERIAL_NUMBER), _))
            .Times(11)
            .WillRepeatedly(DoAll(WriteStringToMapValueArgPointee<2>(TEST_DEVICE_SERIAL_NUMBER), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_SOFTWARE_VERSION_INDEX), _))
            .Times(11)
            .WillRepeatedly(Return(FFS_NOT_IMPLEMENTED));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_FIRMWARE_VERSION), _))
            .Times(11)
            .WillRepeatedly(DoAll(WriteStringToMapValueArgPointee<2>(TEST_DEVICE_FIRMWARE_REVISION), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_,
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_HARDWARE_VERSION), _))
            .Times(11)
            .WillRepeatedly(DoAll(WriteStringToMapValueArgPointee<2>(TEST_DEVICE_HARDWARE_REVISION), Return(FFS_SUCCESS)));

    // Mock 'ffsGetRegistrationDetails'.
    FfsRegistrationDetails_t registrationDetails = FfsRegistrationDetails_t();
    registrationDetails.state = FFS_REGISTRATION_STATE_FAILED;
    registrationDetails.hasHttpCode = true;
    registrationDetails.httpCode = 403;
    EXPECT_COMPAT_CALL(ffsGetRegistrationDetails(getUserContext(), _))
            .Times(7) //!< Reports.
            .WillRepeatedly(DoAll(SetArgPointee<1>(registrationDetails), Return(FFS_SUCCESS)));

    // Mock 'ffsWifiManagerGetConnectionDetails'.
    FfsWifiConnectionDetails_t wifiConnectionDetails = FfsWifiConnectionDetails_t();
    wifiConnectionDetails.state = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;
    EXPECT_COMPAT_CALL(ffsGetWifiConnectionDetails(getUserContext(), _))
            .Times(2) //!< 'CONNECTING_TO_SETUP_NETWORK' and 'CONNECTING_TO_USER_NETWORK'.
            .WillRepeatedly(DoAll(SetArgPointee<1>(wifiConnectionDetails), Return(FFS_SUCCESS)));

    // Mock 'ffsGetWifiConnectionAttempt'.
    FfsWifiConnectionDetails_t setupNetworkConnectionAttempt;
    memset(&setupNetworkConnectionAttempt, 0, sizeof(setupNetworkConnectionAttempt));
    setupNetworkConnectionAttempt.ssidStream = FFS_STRING_INPUT_STREAM("simple_setup");
    setupNetworkConnectionAttempt.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
    setupNetworkConnectionAttempt.state = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;
    FfsWifiConnectionDetails_t wifiConnectionAttempt;
    memset(&wifiConnectionAttempt, 0, sizeof(wifiConnectionAttempt));
    wifiConnectionAttempt.ssidStream = FFS_STRING_INPUT_STREAM("TEST_SSID");
    wifiConnectionAttempt.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
    wifiConnectionAttempt.state = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;
    EXPECT_COMPAT_CALL(ffsGetWifiConnectionAttempt(_, _, _))
            // Setup network.
            .WillOnce(DoAll(SetArgPointee<1>(setupNetworkConnectionAttempt), SetArgPointee<2>(false), Return(FFS_SUCCESS)))
            .WillOnce(DoAll(SetArgPointee<2>(true), Return(FFS_SUCCESS)))
            // User network.
            .WillOnce(DoAll(SetArgPointee<1>(wifiConnectionAttempt), SetArgPointee<2>(false), Return(FFS_SUCCESS)))
            .WillOnce(DoAll(SetArgPointee<2>(true), Return(FFS_SUCCESS)));

    // Mock 'ffsSha256'. Return "not implemented" to trigger the test SHA256 implementation.
    EXPECT_COMPAT_CALL(ffsSha256(_, SaltedPinMatches(TEST_SALTED_DEVICE_PIN), _))
            .WillOnce(Return(FFS_NOT_IMPLEMENTED));

    // Start the task.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'NOT_PROVISIONED'.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_SETUP_NETWORK))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'CONNECTING_TO_SETUP_NETWORK'.
    EXPECT_COMPAT_CALL(ffsAddWifiConfiguration(getUserContext(), _))
            .WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsConnectToWifi(getUserContext()))
            .Times(2) //!< 'CONNECTING_TO_SETUP_NETWORK' and 'CONNECTING_TO_USER_NETWORK'.
            .WillRepeatedly(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_START_PROVISIONING))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'START_PROVISIONING'.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'START_PIN_BASED_SETUP'.
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(_, FFS_CONFIGURATION_ENTRY_KEY_PIN, _))
            .WillOnce(DoAll(WriteStringToMapValueArgPointee<2>(TEST_DEVICE_PIN), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'COMPUTE_CONFIGURATION'.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'POST_WIFI_SCAN_DATA'.
    FFS_LITERAL_INPUT_STREAM(bssidStream, {0x00, 0x11, 0x22, 0x33, 0x44, 0x55});
    FfsWifiScanResult_t wifiScanResult;
    wifiScanResult.ssidStream = FFS_STRING_INPUT_STREAM("TEST_SSID");
    wifiScanResult.bssidStream = bssidStream;
    wifiScanResult.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
    wifiScanResult.frequencyBand = 0;
    wifiScanResult.signalStrength = 0;
    EXPECT_COMPAT_CALL(ffsWifiProvisioneeCanPostWifiScanData(getUserContext(), _, _, _, _))
            .WillOnce(DoAll(SetArgPointee<4>(true), Return(FFS_SUCCESS)))
            .WillOnce(DoAll(SetArgPointee<4>(false), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsGetWifiScanResult(getUserContext(), _, _))
            .WillOnce(DoAll(SetArgPointee<1>(wifiScanResult), SetArgPointee<2>(false), Return(FFS_SUCCESS)))
            .WillOnce(DoAll(SetArgPointee<2>(true), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'GET_WIFI_LIST'.
    EXPECT_COMPAT_CALL(ffsWifiProvisioneeCanGetWifiCredentials(getUserContext(), _, _, _))
            .WillOnce(DoAll(SetArgPointee<3>(true), Return(FFS_SUCCESS)))
            .WillOnce(DoAll(SetArgPointee<3>(false), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'CONNECTING_TO_USER_NETWORK'.
    EXPECT_COMPAT_CALL(ffsRemoveWifiConfiguration(getUserContext(), _))
            .WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'CONNECTED_TO_USER_NETWORK'.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_DONE))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Run the task.
    ASSERT_SUCCESS(ffsWifiProvisioneeTask(getUserContext()));
}

TEST_F(TaskTests, TaskCloudCannotProceed)
{
    // Provisionee state.
    FFS_WIFI_PROVISIONEE_STATE state;
    EXPECT_COMPAT_CALL(ffsGetWifiProvisioneeState(getUserContext(), _))
            .Times(3) //!< Expect to handle 3 states.
            .WillRepeatedly(DoAll(SetArgPointee<1>(ByRef(state)), Return(FFS_SUCCESS)));

    // Mock 'ffsWifiProvisioneeCanProceed'.
    EXPECT_COMPAT_CALL(ffsWifiProvisioneeCanProceed(getUserContext(), _))
            .Times(3) //!< Expect to handle 3 states.
            .WillRepeatedly(DoAll(SetArgPointee<1>(true), Return(FFS_SUCCESS)));

    // Mock 'ffsDssClientGetBuffers'.
    FFS_TEMPORARY_OUTPUT_STREAM(hostStream, DSS_HOST_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(sessionIdStream, DSS_SESSION_ID_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(nonceStream, DSS_NONCE_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, DSS_BODY_BUFFER_SIZE);
    EXPECT_COMPAT_CALL(ffsDssClientGetBuffers(getUserContext(), _, _, _, _)).WillOnce(DoAll(
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
            .WillRepeatedly(DoAll(SetArgPointee<3>(true),
                    Return(FFS_SUCCESS)));

    // Mock 'ffsHttpExecute'.
    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), _, _))
            // 'START_PROVISIONING'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendStartProvisioningSessionBodyCannotProceed),
                    Return(FFS_SUCCESS)));

    // Mock 'ffsGetRandomBytes'.
    EXPECT_COMPAT_CALL(ffsRandomBytes(_, PointeeSpaceIs(strlen(TEST_RANDOM))))
            .WillRepeatedly(DoAll(WriteStringToArgPointee<1>(TEST_RANDOM), Return(FFS_SUCCESS)));

    // Do not involve encoded SSID for this test.
    // Make the first call not return a success reponse to bypass the calculation.
    EXPECT_COMPAT_CALL(ffsRandomBytes(getUserContext(), PointeeSpaceIs(ENCODED_SSID_NONCE_SIZE)))
        .WillOnce(Return(FFS_NOT_IMPLEMENTED));

    // Mock 'ffsGetSetupNetworkConfiguration'.
    EXPECT_COMPAT_CALL(ffsGetSetupNetworkConfiguration(getUserContext(), _))
            .WillOnce(Return(FFS_NOT_IMPLEMENTED)); 

    // Mock 'ffsWifiManagerGetConnectionDetails'.
    FfsWifiConnectionDetails_t wifiConnectionDetails = FfsWifiConnectionDetails_t();
    wifiConnectionDetails.state = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;
    EXPECT_COMPAT_CALL(ffsGetWifiConnectionDetails(getUserContext(), _))
            .WillOnce(DoAll(SetArgPointee<1>(wifiConnectionDetails), Return(FFS_SUCCESS)));

    // Start the task.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'NOT_PROVISIONED'.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_SETUP_NETWORK))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'CONNECTING_TO_SETUP_NETWORK'.
    EXPECT_COMPAT_CALL(ffsAddWifiConfiguration(getUserContext(), _))
            .WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsConnectToWifi(getUserContext())).WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_START_PROVISIONING))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Run the task.
    ASSERT_SUCCESS(ffsWifiProvisioneeTask(getUserContext()));
}

TEST_F(TaskTests, TaskClientCannotProceed)
{
    // Provisionee state.
    FFS_WIFI_PROVISIONEE_STATE state;
    EXPECT_COMPAT_CALL(ffsGetWifiProvisioneeState(getUserContext(), _))
            .WillOnce(DoAll(SetArgPointee<1>(ByRef(state)), Return(FFS_SUCCESS)));

    // Mock 'ffsWifiProvisioneeCanProceed'.
    EXPECT_COMPAT_CALL(ffsWifiProvisioneeCanProceed(getUserContext(), _))
            .WillOnce(DoAll(SetArgPointee<1>(true), Return(FFS_SUCCESS))) // !< Execute one states.
            .WillOnce(DoAll(SetArgPointee<1>(false), Return(FFS_SUCCESS)));

    // Mock 'ffsDssClientGetBuffers'.
    FFS_TEMPORARY_OUTPUT_STREAM(hostStream, DSS_HOST_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(sessionIdStream, DSS_SESSION_ID_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(nonceStream, DSS_NONCE_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, DSS_BODY_BUFFER_SIZE);
    EXPECT_COMPAT_CALL(ffsDssClientGetBuffers(getUserContext(), _, _, _, _)).WillOnce(DoAll(
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
            .WillRepeatedly(DoAll(SetArgPointee<3>(true),
                    Return(FFS_SUCCESS)));

    // Do not involve encoded SSID for this test.
    // Make the first call not return a success reponse to bypass the calculation.
    EXPECT_COMPAT_CALL(ffsRandomBytes(getUserContext(), PointeeSpaceIs(ENCODED_SSID_NONCE_SIZE)))
        .WillOnce(Return(FFS_NOT_IMPLEMENTED));

    // Mock 'ffsGetRandomBytes'.
    EXPECT_COMPAT_CALL(ffsRandomBytes(_, PointeeSpaceIs(strlen(TEST_RANDOM))))
            .WillRepeatedly(DoAll(WriteStringToArgPointee<1>(TEST_RANDOM), Return(FFS_SUCCESS)));

    // Start the task.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'NOT_PROVISIONED'.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_SETUP_NETWORK))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Run the task.
    ASSERT_SUCCESS(ffsWifiProvisioneeTask(getUserContext()));
}

TEST_F(TaskTests, TaskCannotFindEncodedNetworkWithCloudCannotProceed)
{
    // Provisionee state.
    FFS_WIFI_PROVISIONEE_STATE state;
    EXPECT_COMPAT_CALL(ffsGetWifiProvisioneeState(getUserContext(), _))
            .Times(3) //!< Expect to handle 3 states.
            .WillRepeatedly(DoAll(SetArgPointee<1>(ByRef(state)), Return(FFS_SUCCESS)));

    // Mock 'ffsWifiProvisioneeCanProceed'.
    EXPECT_COMPAT_CALL(ffsWifiProvisioneeCanProceed(getUserContext(), _))
            .Times(3) //!< Expect to handle 3 states.
            .WillRepeatedly(DoAll(SetArgPointee<1>(true), Return(FFS_SUCCESS)));

    // Mock 'ffsDssClientGetBuffers'.
    FFS_TEMPORARY_OUTPUT_STREAM(hostStream, DSS_HOST_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(sessionIdStream, DSS_SESSION_ID_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(nonceStream, DSS_NONCE_BUFFER_SIZE);
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, DSS_BODY_BUFFER_SIZE);
    EXPECT_COMPAT_CALL(ffsDssClientGetBuffers(getUserContext(), _, _, _, _)).WillOnce(DoAll(
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
            .WillRepeatedly(DoAll(SetArgPointee<3>(true),
                    Return(FFS_SUCCESS)));

    // Mock 'ffsHttpExecute'.
    EXPECT_COMPAT_CALL(ffsHttpExecute(getUserContext(), _, _))
            // 'START_PROVISIONING'.
            .WillOnce(DoAll(Invoke(sendSignatureResponse),
                    Invoke(sendStartProvisioningSessionBodyCannotProceed),
                    Return(FFS_SUCCESS)));

    // Mock 'ffsGetRandomBytes'.
    EXPECT_COMPAT_CALL(ffsRandomBytes(_, PointeeSpaceIs(strlen(TEST_RANDOM))))
            .WillRepeatedly(DoAll(WriteStringToArgPointee<1>(TEST_RANDOM), Return(FFS_SUCCESS)));

    // Mock the encoded SSID/passphrase related calls.
    FFS_LITERAL_INPUT_STREAM(randomNonceForSSIDStream, TEST_12_BYTE_NONCE);
    EXPECT_COMPAT_CALL(ffsRandomBytes(getUserContext(), PointeeSpaceIsSizeOf(randomNonceForSSIDStream)))
            .WillOnce(DoAll(WriteStreamToArgPointee<1>(randomNonceForSSIDStream), Return(FFS_SUCCESS)));

    // Mock public key.
    FFS_LITERAL_INPUT_STREAM(testPublicKeyStream, TEST_PUBLIC_KEY_DER_BYTES);
    FFS_LITERAL_INPUT_STREAM(testHashedPublicKeyStream, TEST_HASH_BYTES);
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(getUserContext(),
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_DEVICE_EC_PUBLIC_KEY_DER), _))
            .WillOnce(DoAll(WriteByteStreamToMapValueArgPointee<2>(testPublicKeyStream), Return(FFS_SUCCESS)));
    EXPECT_COMPAT_CALL(ffsSha256(_, PublicKeyMatches(testPublicKeyStream), _))
            .WillOnce(DoAll(WriteStreamToArgPointee<2>(testHashedPublicKeyStream), Return(FFS_SUCCESS)));

    // Mock product index.
    FFS_LITERAL_INPUT_STREAM(productIndexStream, { 0x11, 0x22, 0x33, 0x44});
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(getUserContext(),
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_PRODUCT_INDEX), _))
            .WillOnce(DoAll(WriteStringStreamToMapValueArgPointee<2>(productIndexStream), Return(FFS_SUCCESS)));

    // Mock cloud public key.
    EXPECT_COMPAT_CALL(ffsGetConfigurationValue(getUserContext(),
            StrEq(FFS_CONFIGURATION_ENTRY_KEY_CLOUD_EC_PUBLIC_KEY_DER), _))
            .WillOnce(DoAll(WriteByteStreamToMapValueArgPointee<2>(testPublicKeyStream), Return(FFS_SUCCESS)));

    // Mock ECDH shared secret.
    EXPECT_COMPAT_CALL(ffsComputeECDHKey(getUserContext(), PointeeStreamEq(testPublicKeyStream), PointeeSpaceIsSizeOf(testHashedPublicKeyStream)))
            .WillOnce(DoAll(WriteStreamToArgPointee<2>(testHashedPublicKeyStream), Return(FFS_SUCCESS)));

    // Mock HMAC calculation.
    EXPECT_COMPAT_CALL(ffsComputeHMACSHA256(getUserContext(), PointeeStreamEq(testHashedPublicKeyStream),
            PointeeStreamEq(randomNonceForSSIDStream), PointeeSpaceIsSizeOf(testHashedPublicKeyStream)))
                    .WillOnce(DoAll(WriteStreamToArgPointee<3>(testHashedPublicKeyStream), Return(FFS_SUCCESS)));

    // Mock 'ffsGetSetupNetworkConfiguration'.
    EXPECT_COMPAT_CALL(ffsGetSetupNetworkConfiguration(getUserContext(), _))
            .WillOnce(Return(FFS_NOT_IMPLEMENTED)); 

    // Mock 'ffsWifiManagerGetConnectionDetails'.
    FfsWifiConnectionDetails_t wifiConnectionDetailsFailed = FfsWifiConnectionDetails_t();
    wifiConnectionDetailsFailed.state = FFS_WIFI_CONNECTION_STATE_FAILED;
    FfsWifiConnectionDetails_t wifiConnectionDetailsAssociated = FfsWifiConnectionDetails_t();
    wifiConnectionDetailsAssociated.state = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;

    EXPECT_COMPAT_CALL(ffsGetWifiConnectionDetails(getUserContext(), _))
            .WillOnce(DoAll(SetArgPointee<1>(wifiConnectionDetailsFailed), Return(FFS_SUCCESS))) //!< Failed to connect to first encoded setup network.
            .WillOnce(DoAll(SetArgPointee<1>(wifiConnectionDetailsAssociated), Return(FFS_SUCCESS))); //!< Second time success

    // Start the task.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'NOT_PROVISIONED'.
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_SETUP_NETWORK))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Execute 'CONNECTING_TO_SETUP_NETWORK'.
    EXPECT_COMPAT_CALL(ffsAddWifiConfiguration(getUserContext(), _))
            .Times(2) //!< Once for encoded, once for default.
            .WillRepeatedly(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsConnectToWifi(getUserContext()))
            .Times(2) //!< Once for encoded, once for default.
            .WillRepeatedly(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsSetWifiProvisioneeState(getUserContext(), FFS_WIFI_PROVISIONEE_STATE_START_PROVISIONING))
            .WillOnce(DoAll(SaveArg<1>(&state), Return(FFS_SUCCESS)));

    // Run the task.
    ASSERT_SUCCESS(ffsWifiProvisioneeTask(getUserContext()));
}
