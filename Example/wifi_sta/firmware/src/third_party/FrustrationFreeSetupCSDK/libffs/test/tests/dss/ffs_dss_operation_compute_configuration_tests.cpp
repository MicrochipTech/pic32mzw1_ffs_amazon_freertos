/** @file ffs_dss_operation_compute_configuration_tests.cpp
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
#include "ffs/conversion/ffs_convert_registration_details.h"
#include "constants/test_constants.h"
#include "helpers/test_utilities.h"
#include "ffs/dss/ffs_dss_operation_compute_configuration_data.h"

#define DSS_SIGNATURE_HEADER_KEY        "x-amzn-dss-signature"
#define DSS_SIGNATURE_HEADER_VALUE      "SIGNATURE"

// Static functions.
extern "C" {
static FFS_RESULT saveRegistrationDetailsCallback(struct FfsUserContext_s *userContext,
        FfsDssRegistrationDetails_t *registrationDetails);
static FFS_RESULT saveConfigurationCallback(struct FfsUserContext_s *userContext,
        const char *key, FfsMapValue_t *value);
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

/** @brief Registration request matcher.
 */
MATCHER_P(RegistrationRequestMatches, sourceRegistrationRequest, "Registration request matches")
{
    return ffsStreamMatchesStream(&arg->tokenStream, (FfsStream_t *)&sourceRegistrationRequest.tokenStream);
}

/** @brief Configuration entry key matcher.
 */
MATCHER_P(ConfigurationEntryMatches, sourceKey, "Configuration entry matches")
{
    return ffsStreamMatchesString(&arg->keyStream, sourceKey);
}

class DssComputeConfigurationTests: public TestContextFixture {
public:

    /** @brief Test the request callbacks with mock data.
     */
    static void handleComputeConfigurationResponse(struct FfsUserContext_s *userContext,
            FfsHttpRequest_t *request, void *callbackDataPointer) {
        (void) userContext;

        FfsHttpCallbacks_t callbacks = request->callbacks;

        const char *MOCK_RESPONSE_BODY_STRING = "{"
                "\"nonce\":\"" TEST_NONCE "\","
                "\"configuration\":"
                "{"
                    "\"LocaleConfiguration.LanguageLocale\":\"" TEST_LANGUAGE_LOCALE "\","
                    "\"LocaleConfiguration.CountryCode\":\"" TEST_COUNTRY_CODE "\","
                    "\"LocaleConfiguration.CountryOfResidence\":\"" TEST_COUNTRY_OF_RESIDENCE "\","
                    "\"LocaleConfiguration.Region\":\"" TEST_REGION "\","
                    "\"LocaleConfiguration.Realm\":\"" TEST_REALM "\","
                    "\"LocaleConfiguration.Marketplace\":\"" TEST_MARKETPLACE "\","
                    "\"UNSUPPORTED\":\"CONFIGURATION\""
                "},"
                "\"registrationDetails\":"
                "{"
                    "\"registrationToken\":\"" TEST_REGISTRATION_TOKEN "\","
                    "\"expiresAt\":" TEST_EXPIRES_AT
                "}"
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

    /** brief Ffs string map value constructor.
     */
    FfsMapValue_t stringMapValueConstructor(const char *value) {
        FfsMapValue_t mapValue;
        mapValue.type = FFS_MAP_VALUE_TYPE_STRING;
        mapValue.stringStream = FFS_STRING_INPUT_STREAM(value);
        return mapValue;
    }

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

/** @brief Test a 'compute configuration' DSS operation.
 */
TEST_F(DssComputeConfigurationTests, ComputeConfiguration)
{
    // DSS client context.
    FFS_TEMPORARY_OUTPUT_STREAM(hostStream, 256);
    FFS_TEMPORARY_OUTPUT_STREAM(sessionIdStream, 256);
    FFS_TEMPORARY_OUTPUT_STREAM(nonceStream, 16);
    FFS_TEMPORARY_OUTPUT_STREAM(bodyStream, 1024);
    FfsDssClientContext_t dssClientContext;
    EXPECT_COMPAT_CALL(ffsDssClientGetBuffers(getUserContext(), _, _, _, _))
            .WillOnce(DoAll(SetArgPointee<1>(hostStream),
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
            "}"
            "}";

    // Expected registration request.
    FfsRegistrationRequest_t EXPECTED_REGISTRATION_REQUEST = ffsRegistrationRequestConstructor(TEST_REGISTRATION_TOKEN, TEST_EXPIRES_AT_VALUE);

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
            .WillOnce(DoAll(Invoke(handleComputeConfigurationResponse), Return(FFS_SUCCESS)));

    // Mock 'ffsSetRegistrationToken'.
    EXPECT_COMPAT_CALL(ffsSetRegistrationToken(_, RegistrationRequestMatches(EXPECTED_REGISTRATION_REQUEST)))
            .WillOnce(Return(FFS_SUCCESS));

    // Mock 'ffsSetConfigurationValue'.
    EXPECT_COMPAT_CALL(ffsSetConfigurationValue(_, StrEq(FFS_CONFIGURATION_ENTRY_KEY_LANGUAGE_LOCALE), _))
            .WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsSetConfigurationValue(_, StrEq(FFS_CONFIGURATION_ENTRY_KEY_COUNTRY_CODE), _))
            .WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsSetConfigurationValue(_, StrEq(FFS_CONFIGURATION_ENTRY_KEY_COUNTRY_OF_RESIDENCE), _))
            .WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsSetConfigurationValue(_, StrEq(FFS_CONFIGURATION_ENTRY_KEY_REGION), _))
            .WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsSetConfigurationValue(_, StrEq(FFS_CONFIGURATION_ENTRY_KEY_REALM), _))
            .WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsSetConfigurationValue(_, StrEq(FFS_CONFIGURATION_ENTRY_KEY_MARKETPLACE), _))
            .WillOnce(Return(FFS_SUCCESS));
    EXPECT_COMPAT_CALL(ffsSetConfigurationValue(_, StrEq("UNSUPPORTED"), _))
            .WillOnce(Return(FFS_NOT_IMPLEMENTED));

    // Execute the operation.
    ASSERT_SUCCESS(ffsDssComputeConfigurationData(&dssClientContext,
            saveRegistrationDetailsCallback,
            saveConfigurationCallback));
}

extern "C" {

/** @brief Callback to save the registration details.
 *
 * @param userContext User context
 * @param registrationDetails Registration details to save
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT saveRegistrationDetailsCallback(struct FfsUserContext_s *userContext,
        FfsDssRegistrationDetails_t *registrationDetails)
{
    FfsRegistrationRequest_t registrationRequest;
    FFS_CHECK_RESULT(ffsConvertDssRegistrationDetailsToApi(registrationDetails, &registrationRequest));
    FFS_CHECK_RESULT(ffsSetRegistrationToken(userContext, &registrationRequest));

    return FFS_SUCCESS;
}

/** @brief Callback to save a configuration entry.
 *
 * @param userContext User context
 * @param key Configuration key
 * @param value Configuration value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
static FFS_RESULT saveConfigurationCallback(struct FfsUserContext_s *userContext,
        const char *key, FfsMapValue_t *value)
{
    FFS_CHECK_RESULT(ffsSetConfigurationValue(userContext, key, value));

    return FFS_SUCCESS;
}

}
