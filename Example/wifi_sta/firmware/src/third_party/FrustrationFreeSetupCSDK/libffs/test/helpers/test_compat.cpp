/** @file test_compat.cpp
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

#include "test_context.h"

#include "sha-256/sha-256.h"

#include "ffs/common/ffs_check_result.h"

#include <stdarg.h>

static FFS_LOG_LEVEL currentLogLevel = FFS_LOG_LEVEL_DEBUG;

/*
 * Set the current log level.
 */
void ffsSetLogLevel(FFS_LOG_LEVEL logLevel)
{
    currentLogLevel = logLevel;
}

extern "C" {

/*
 * Log a message.
 */
FFS_RESULT ffsLog(FFS_LOG_LEVEL logLevel, const char *functionName, int lineNumber, const char *format, ...)
{
#if !defined(FFS_DEBUG)

    (void) logLevel;
    (void) functionName;
    (void) lineNumber;
    (void) format;

#else

    if (logLevel < currentLogLevel) {
        return FFS_SUCCESS;
    }

    // Print the log level.
    switch (logLevel) {
        case FFS_LOG_LEVEL_DEBUG:
            printf("[DEBUG]");
            break;
        case FFS_LOG_LEVEL_INFO:
            printf("[INFO]");
            break;
        case FFS_LOG_LEVEL_WARNING:
            printf("[WARNING]");
            break;
        case FFS_LOG_LEVEL_ERROR:
            printf("[ERROR]");
            break;
        default:
            FFS_FAIL(FFS_ERROR);
    }

    printf(" ");

    // Print the function name and line number.
    if (functionName) {
        printf("%s:%d: ", functionName, lineNumber);
    }

    // Print the data.
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    // Terminate the line.
    printf("\n");

#endif

    return FFS_SUCCESS;
}

/*
 * Generate a sequence of random bytes.
 */
FFS_RESULT ffsRandomBytes(struct FfsUserContext_s *userContext, FfsStream_t *randomStream)
{
    return userContext->compat.ffsRandomBytes(userContext, randomStream);
}

/*
 * Calculate a SHA256 hash.
 */
FFS_RESULT ffsSha256(struct FfsUserContext_s *userContext, FfsStream_t *dataStream,
        FfsStream_t *hashStream)
{
    (void) userContext;

    // Mocks can return "not implemented" to use the default hash implementation.
    FFS_RESULT result = userContext->compat.ffsSha256(userContext, dataStream, hashStream);
    if (result != FFS_NOT_IMPLEMENTED) {
        return result;
    }

    // Do we have enough space for the hash?
    if (FFS_STREAM_SPACE_SIZE(*hashStream) < 32) {
        FFS_FAIL(FFS_OVERRUN);
    }

    // Calculate the hash.
    calc_sha_256(FFS_STREAM_NEXT_WRITE(*hashStream), FFS_STREAM_NEXT_READ(*dataStream), FFS_STREAM_DATA_SIZE(*dataStream));

    // Update the streams.
    FFS_CHECK_RESULT(ffsReadStream(dataStream, FFS_STREAM_DATA_SIZE(*dataStream), NULL));
    FFS_CHECK_RESULT(ffsWriteStream(NULL, 32, hashStream));

    return FFS_SUCCESS;
}

/*
 * Calculate a HMAC
 */
FFS_RESULT ffsComputeHMACSHA256(struct FfsUserContext_s *userContext, FfsStream_t *secretKeyStream, FfsStream_t *dataStream,
        FfsStream_t *hmacStream)
{
    return userContext->compat.ffsComputeHMACSHA256(userContext, secretKeyStream, dataStream, hmacStream);
}

/*
 * Calculate a ECDH shared Secret
 */
FFS_RESULT ffsComputeECDHKey(struct FfsUserContext_s *userContext, FfsStream_t *publicKeyStream, FfsStream_t *secretKeyStream)
{
    return userContext->compat.ffsComputeECDHKey(userContext, publicKeyStream, secretKeyStream);
}

/*
 * Set a configuration value.
 */
FFS_RESULT ffsSetConfigurationValue(struct FfsUserContext_s *userContext,
        const char *configurationKey, FfsMapValue_t *configurationValue)
{
    return userContext->compat.ffsSetConfigurationValue(userContext, configurationKey, configurationValue);
}

/*
 * Get a configuration value.
 */
FFS_RESULT ffsGetConfigurationValue(struct FfsUserContext_s *userContext,
        const char *configurationKey, FfsMapValue_t *configurationValue)
{
    return userContext->compat.ffsGetConfigurationValue(userContext, configurationKey, configurationValue);
}

/*
 * Set the registration token (session ID).
 */
FFS_RESULT ffsSetRegistrationToken(struct FfsUserContext_s *userContext,
        FfsRegistrationRequest_t *registrationRequest)
{
    return userContext->compat.ffsSetRegistrationToken(userContext, registrationRequest);
}

/*
 * Get the registraiton token (session ID).
 */
FFS_RESULT ffsGetRegistrationToken(struct FfsUserContext_s *userContext,
        FfsStream_t *tokenStream)
{
    return userContext->compat.ffsGetRegistrationToken(userContext, tokenStream);
}

/*
 * Get the current registration status.
 */
FFS_RESULT ffsGetRegistrationDetails(struct FfsUserContext_s *userContext,
        FfsRegistrationDetails_t *registrationDetails)
{
    return userContext->compat.ffsGetRegistrationDetails(userContext, registrationDetails);
}

/*
 * Verify a cloud signature.
 */
FFS_RESULT ffsVerifyCloudSignature(struct FfsUserContext_s *userContext,
        FfsStream_t *payloadStream, FfsStream_t *signatureStream, bool *isVerified)
{
    return userContext->compat.ffsVerifyCloudSignature(userContext, payloadStream, signatureStream, isVerified);
}

/*
 * Execute an HTTP operation.
 */
FFS_RESULT ffsHttpExecute(struct FfsUserContext_s *userContext,
        FfsHttpRequest_t *request, void *callbackDataPointer)
{
    return userContext->compat.ffsHttpExecute(userContext, request, callbackDataPointer);
}

/*
 * Get the next Wi-Fi scan result.
 */
FFS_RESULT ffsGetWifiScanResult(struct FfsUserContext_s *userContext, FfsWifiScanResult_t *wifiScanResult,
        bool *isUnderrun)
{
    return userContext->compat.ffsGetWifiScanResult(userContext, wifiScanResult, isUnderrun);
}

/*
 * Provide Wi-Fi network credentials to the client.
 */
FFS_RESULT ffsAddWifiConfiguration(struct FfsUserContext_s *userContext, FfsWifiConfiguration_t
        *wifiConfiguration)
{
    return userContext->compat.ffsAddWifiConfiguration(userContext, wifiConfiguration);
}

/*
 * Remove a stored Wi-Fi configuration.
 */
FFS_RESULT ffsRemoveWifiConfiguration(struct FfsUserContext_s *userContext, FfsStream_t ssidStream)
{
    return userContext->compat.ffsRemoveWifiConfiguration(userContext, ssidStream);
}

/*
 * Start connecting to the stored Wi-Fi network(s).
 */
FFS_RESULT ffsConnectToWifi(struct FfsUserContext_s *userContext)
{
    return userContext->compat.ffsConnectToWifi(userContext);
}

/*
 * Get the current Wi-Fi connection state.
 */
FFS_RESULT ffsGetWifiConnectionDetails(struct FfsUserContext_s *userContext, FfsWifiConnectionDetails_t
        *wifiConnectionDetails)
{
    return userContext->compat.ffsGetWifiConnectionDetails(userContext, wifiConnectionDetails);
}

/*
 * Get the list of connection attempts for the current session.
 */
FFS_RESULT ffsGetWifiConnectionAttempt(struct FfsUserContext_s *userContext, FfsWifiConnectionAttempt_t
        *wifiConnectionAttempt, bool *isUnderrun)
{
    return userContext->compat.ffsGetWifiConnectionAttempt(userContext, wifiConnectionAttempt, isUnderrun);
}

/*
 * Get DSS client buffers.
 */
FFS_RESULT ffsDssClientGetBuffers(struct FfsUserContext_s *userContext,
        FfsStream_t *hostStream, FfsStream_t *sessionIdStream,
        FfsStream_t *nonceStream, FfsStream_t *bodyStream)
{
    return userContext->compat.ffsDssClientGetBuffers(userContext, hostStream, sessionIdStream, nonceStream, bodyStream);
}

/*
 * Calculate an deterministic ECDSA signature using v2 DHA private key.
 */
FFS_RESULT ffsDeterministicEcdsaWithDhaPrivateKey(struct FfsUserContext_s *userContext,
        FfsStream_t *hashStream, FfsStream_t *pointRStream, FfsStream_t *pointSStream)
{
    return userContext->compat.ffsDeterministicEcdsaWithDhaPrivateKey(userContext, hashStream, pointRStream,
            pointSStream);
}

/*
 * Set the state of the provisionee.
 */
FFS_RESULT ffsSetWifiProvisioneeState(struct FfsUserContext_s *userContext,
        FFS_WIFI_PROVISIONEE_STATE provisioneeState)
{
    return userContext->compat.ffsSetWifiProvisioneeState(userContext, provisioneeState);
}

/*
 * Get the state of the provisionee.
 */
FFS_RESULT ffsGetWifiProvisioneeState(struct FfsUserContext_s *userContext,
        FFS_WIFI_PROVISIONEE_STATE *provisioneeState)
{
    return userContext->compat.ffsGetWifiProvisioneeState(userContext, provisioneeState);
}

/*
 * Gets the client's custom setup network configuration, if one exists.
 */
FFS_RESULT ffsGetSetupNetworkConfiguration(struct FfsUserContext_s *userContext,
        FfsWifiConfiguration_t *wifiConfiguration)
{
    return userContext->compat.ffsGetSetupNetworkConfiguration(userContext, wifiConfiguration);
}

/*
 * Let the client kill the Wi-Fi provisionee task with a boolean value.
 */
FFS_RESULT ffsWifiProvisioneeCanProceed(struct FfsUserContext_s *userContext,
        bool *canProceed)
{
    return userContext->compat.ffsWifiProvisioneeCanProceed(userContext, canProceed);
}

/*
 * Let the client control whether the Wi-Fi provisionee task continues to post Wi-Fi scan data.
 **/
FFS_RESULT ffsWifiProvisioneeCanPostWifiScanData(struct FfsUserContext_s *userContext,
        uint32_t sequenceNumber, uint32_t totalCredentialsFound, bool allCredentialsFound, bool *canPostWifiScanData)
{
    return userContext->compat.ffsWifiProvisioneeCanPostWifiScanData(userContext, sequenceNumber,
            totalCredentialsFound, allCredentialsFound, canPostWifiScanData);
}

/*
 * Let the client control whether the Wi-Fi provisionee task continues to get Wi-Fi credentials.
 */
FFS_RESULT ffsWifiProvisioneeCanGetWifiCredentials(struct FfsUserContext_s *userContext,
        uint32_t sequenceNumber, bool allCredentialsReturned, bool *canGetWifiCredentials)
{
    return userContext->compat.ffsWifiProvisioneeCanGetWifiCredentials(userContext, sequenceNumber,
            allCredentialsReturned, canGetWifiCredentials);
}

}
