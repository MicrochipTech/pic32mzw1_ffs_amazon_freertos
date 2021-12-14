/** @file test_compat.h
 *
 * @copyright 2018 Amazon.com, Inc. or its affiliates.  All rights reserved.
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

#ifndef TEST_COMPAT_H_
#define TEST_COMPAT_H_

#include "ffs/compat/ffs_common_compat.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"

#include <gmock/gmock.h>

/** @brief Set the current log level.
 */
void ffsSetLogLevel(FFS_LOG_LEVEL logLevel);

/** @brief Mockable compatibility layer.
 */
class TestCompat {
public:

    virtual ~TestCompat()
    {
    }

    // Abstract "common C SDK" compatibility-layer functions.
    virtual FFS_RESULT ffsRandomBytes(struct FfsUserContext_s *userContext,
            FfsStream_t *randomStream) = 0;
    virtual FFS_RESULT ffsSha256(struct FfsUserContext_s *userContext, FfsStream_t *dataStream, FfsStream_t *hashStream) = 0;
    virtual FFS_RESULT ffsComputeHMACSHA256(struct FfsUserContext_s *userContext, FfsStream_t *secretKeyStream, FfsStream_t *dataStream, FfsStream_t *hmacStream) = 0;
    virtual FFS_RESULT ffsComputeECDHKey(struct FfsUserContext_s *userContext, FfsStream_t *publicKeyStream, FfsStream_t *secretKeyStream) = 0;
    virtual FFS_RESULT ffsSetConfigurationValue(struct FfsUserContext_s *userContext,
            const char *configurationKey, FfsMapValue_t *configurationValue) = 0;
    virtual FFS_RESULT ffsGetConfigurationValue(struct FfsUserContext_s *userContext,
            const char *configurationKey, FfsMapValue_t *configurationValue) = 0;
    virtual FFS_RESULT ffsSetRegistrationToken(struct FfsUserContext_s *userContext,
            FfsRegistrationRequest_t *registrationRequest) = 0;
    virtual FFS_RESULT ffsGetRegistrationToken(struct FfsUserContext_s *userContext,
            FfsStream_t *tokenStream) = 0;
    virtual FFS_RESULT ffsGetRegistrationDetails(struct FfsUserContext_s *userContext,
            FfsRegistrationDetails_t *registrationDetails) = 0;
    virtual FFS_RESULT ffsVerifyCloudSignature(struct FfsUserContext_s *userContext,
            FfsStream_t *payloadStream, FfsStream_t *signatureStream, bool *isVerified) = 0;
    virtual FFS_RESULT ffsHttpExecute(struct FfsUserContext_s *userContext,
            FfsHttpRequest_t *request, void *callbackDataPointer) = 0;
    virtual FFS_RESULT ffsGetWifiScanResult(struct FfsUserContext_s *userContext,
            FfsWifiScanResult_t *wifiScanResult, bool *isUnderrun) = 0;
    virtual FFS_RESULT ffsAddWifiConfiguration(struct FfsUserContext_s *userContext,
            FfsWifiConfiguration_t *wifiConfiguration) = 0;
    virtual FFS_RESULT ffsRemoveWifiConfiguration(struct FfsUserContext_s *userContext,
            FfsStream_t ssidStream) = 0;
    virtual FFS_RESULT ffsConnectToWifi(struct FfsUserContext_s *userContext) = 0;
    virtual FFS_RESULT ffsGetWifiConnectionDetails(struct FfsUserContext_s *userContext,
            FfsWifiConnectionDetails_t *wifiConnectionDetails) = 0;
    virtual FFS_RESULT ffsGetWifiConnectionAttempt(struct FfsUserContext_s *userContext,
            FfsWifiConnectionAttempt_t *wifiConnectionAttempt, bool *isUnderrun) = 0;

    // Abstract "DSS client" compatability-layer functions.
    virtual FFS_RESULT ffsDssClientGetBuffers(struct FfsUserContext_s *userContext,
            FfsStream_t *hostStream, FfsStream_t *sessionIdStream,
            FfsStream_t *nonceStream, FfsStream_t *bodyStream) = 0;

    // Abstract "Wi-Fi provisionee SDK" compatibility-layer functions.
    virtual FFS_RESULT ffsDeterministicEcdsaWithDhaPrivateKey(struct FfsUserContext_s *userContext, FfsStream_t *hashStream,
            FfsStream_t *pointRStream, FfsStream_t *pointSStream) = 0;
    virtual FFS_RESULT ffsSetWifiProvisioneeState(struct FfsUserContext_s *userContext,
            FFS_WIFI_PROVISIONEE_STATE provisioneeState) = 0;
    virtual FFS_RESULT ffsGetWifiProvisioneeState(struct FfsUserContext_s *userContext,
            FFS_WIFI_PROVISIONEE_STATE *provisioneeState) = 0;
    virtual FFS_RESULT ffsGetSetupNetworkConfiguration(struct FfsUserContext_s *userContext,
            FfsWifiConfiguration_t *wifiConfiguration) = 0;
    virtual FFS_RESULT ffsWifiProvisioneeCanProceed(struct FfsUserContext_s *userContext,
            bool *canProceed) = 0;
    virtual FFS_RESULT ffsWifiProvisioneeCanPostWifiScanData(struct FfsUserContext_s *userContext,
            uint32_t sequenceNumber, uint32_t totalCredentialsFound, bool allCredentialsFound, bool *canPostWifiScanData) = 0;
    virtual FFS_RESULT ffsWifiProvisioneeCanGetWifiCredentials(struct FfsUserContext_s *userContext,
            uint32_t sequenceNumber, bool allCredentialsReturned, bool *canGetWifiCredentials) = 0;

};

/** @brief Mock compatibility layer.
 */
class MockCompat : public TestCompat {
public:

    virtual ~MockCompat()
    {
    }

    // Mock "common C SDK" compatibility-layer functions.
    MOCK_METHOD2(ffsRandomBytes, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsStream_t *randomStream));
    MOCK_METHOD3(ffsSha256, FFS_RESULT(struct FfsUserContext_s *userContext, FfsStream_t *dataStream, FfsStream_t *hashStream));
    MOCK_METHOD4(ffsComputeHMACSHA256, FFS_RESULT(struct FfsUserContext_s *userContext, FfsStream_t *secretKeyStream, FfsStream_t *dataStream, FfsStream_t *hmacStream));
    MOCK_METHOD3(ffsComputeECDHKey, FFS_RESULT(struct FfsUserContext_s *userContext, FfsStream_t *publicKeyStream, FfsStream_t *secretKeyStream));
    MOCK_METHOD3(ffsSetConfigurationValue, FFS_RESULT(struct FfsUserContext_s *userContext, const char *configurationKey,
            FfsMapValue_t *configurationValue));
    MOCK_METHOD3(ffsGetConfigurationValue, FFS_RESULT(struct FfsUserContext_s *userContext, const char *configurationKey,
            FfsMapValue_t *configurationValue));
    MOCK_METHOD2(ffsSetRegistrationToken, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsRegistrationRequest_t *registrationRequest));
    MOCK_METHOD2(ffsGetRegistrationToken, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsStream_t *tokenStream));
    MOCK_METHOD2(ffsGetRegistrationDetails, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsRegistrationDetails_t *registrationDetails));
    MOCK_METHOD4(ffsVerifyCloudSignature, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsStream_t *payloadStream, FfsStream_t *signatureStream, bool *isVerified));
    MOCK_METHOD3(ffsHttpExecute, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsHttpRequest_t *request, void *callbackDataPointer));
    MOCK_METHOD3(ffsGetWifiScanResult, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsWifiScanResult_t *wifiScanResult, bool *isUnderrun));
    MOCK_METHOD2(ffsAddWifiConfiguration, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsWifiConfiguration_t *wifiConfiguration));
    MOCK_METHOD2(ffsRemoveWifiConfiguration, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsStream_t ssidStream));
    MOCK_METHOD1(ffsConnectToWifi, FFS_RESULT(struct FfsUserContext_s *userContext));
    MOCK_METHOD2(ffsGetWifiConnectionDetails, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsWifiConnectionDetails_t *wifiConnectionDetails));
    MOCK_METHOD3(ffsGetWifiConnectionAttempt, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsWifiConnectionAttempt_t *wifiConnectionAttempt, bool *isUnderrun));

    // Mock "DSS client" compatibility-layer functions.
    MOCK_METHOD5(ffsDssClientGetBuffers, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsStream_t *hostStream, FfsStream_t *sessionIdStream,
            FfsStream_t *nonceStream, FfsStream_t *bodyStream));

    // Mock "Wi-Fi provisionee SDK" compatibility-layer functions.
    MOCK_METHOD4(ffsDeterministicEcdsaWithDhaPrivateKey, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsStream_t *hashStream, FfsStream_t *pointRStream, FfsStream_t *pointSStream));
    MOCK_METHOD2(ffsSetWifiProvisioneeState, FFS_RESULT(struct FfsUserContext_s *userContext,
            FFS_WIFI_PROVISIONEE_STATE provisioneeState));
    MOCK_METHOD2(ffsGetWifiProvisioneeState, FFS_RESULT(struct FfsUserContext_s *userContext,
            FFS_WIFI_PROVISIONEE_STATE *provisioneeState));
    MOCK_METHOD2(ffsGetSetupNetworkConfiguration, FFS_RESULT(struct FfsUserContext_s *userContext,
            FfsWifiConfiguration_t *wifiConfiguration));
    MOCK_METHOD2(ffsWifiProvisioneeCanProceed, FFS_RESULT(struct FfsUserContext_s *userContext,
            bool *canProceed));
    MOCK_METHOD5(ffsWifiProvisioneeCanPostWifiScanData, FFS_RESULT(struct FfsUserContext_s *userContext,
            uint32_t sequenceNumber, uint32_t totalCredentialsFound, bool allCredentialsFound, bool *canPostWifiScanData));
    MOCK_METHOD4(ffsWifiProvisioneeCanGetWifiCredentials, FFS_RESULT(struct FfsUserContext_s *userContext,
            uint32_t sequenceNumber, bool allCredentialsReturned, bool *canGetWifiCredentials));

};

#endif //TEST_COMPAT_H_
