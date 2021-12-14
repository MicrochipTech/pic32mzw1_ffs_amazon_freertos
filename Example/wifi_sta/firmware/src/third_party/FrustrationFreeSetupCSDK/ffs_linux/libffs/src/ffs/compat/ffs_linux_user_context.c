/** @file ffs_linux_user_context.c
 *
 * @brief Ffs Wi-Fi Linux user context implementation
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

#include "ffs/common/ffs_logging.h"
#include "ffs/common/ffs_check_result.h"
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"
#include "ffs/linux/ffs_wifi_scan_list.h"

#include <fcntl.h>
#include <openssl/x509.h>

#define FFS_TASK_WIFI_SEMAPHORE                     "/wj_task_wifi_semaphore"
#define BACKGROUND_WIFI_SCAN_SEMAPHORE_NAME         "/scan_complete_semaphore"

#define DPSS_PUBLIC_KEY_DEFAULT_PATH                "./data/dss_certificates/dpss_public_key.pem"
#define DSS_SERVER_CA_CERTIFICATES_PATH             "./data/dss_certificates/"
#define DSS_CLIENT_CERTIFICATE_PATH                 "./data/device_certificate/certificate.pem"
#define DSS_CLIENT_CERTIFICATE_PRIVATE_KEY_PATH     "./data/device_certificate/private_key.pem"

#define DSS_HOST_NAME_BUFFER_SIZE                   (256)
#define DSS_SESSION_ID_BUFFER_SIZE                  (1024)
#define DSS_NONCE_BUFFER_SIZE                       (32)
#define DSS_BODY_BUFFER_SIZE                        (4096)


static FFS_RESULT ffsInitializeDevicePublicKey(struct FfsUserContext_s *userContext);

/*
 * Initialize the Ffs Wi-Fi Linux user context.
 */
FFS_RESULT ffsInitializeUserContext(FfsUserContext_t *userContext) {
    // Initialize the Wi-Fi context.
    if (ffsInitializeWifiContext(&userContext->wifiContext)) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    // Initialize the configuration map.
    if (ffsInitializeConfigurationMap(&userContext->configurationMap)) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    // Create the provisionee state mutex.
    if (pthread_mutex_init(&userContext->provisioneeStateMutex, NULL)) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    // Create the Ffs Wi-Fi provisionee task semaphore.
    sem_unlink(FFS_TASK_WIFI_SEMAPHORE);
    userContext->ffsTaskWifiSemaphore = sem_open(FFS_TASK_WIFI_SEMAPHORE,
            O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (userContext->ffsTaskWifiSemaphore == SEM_FAILED) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    // Create the Wi-Fi scan semaphore.
    sem_unlink(BACKGROUND_WIFI_SCAN_SEMAPHORE_NAME);
    userContext->backgroundWifiScanSemaphore = sem_open(BACKGROUND_WIFI_SCAN_SEMAPHORE_NAME,
            O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (userContext->backgroundWifiScanSemaphore == SEM_FAILED) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    // Default to no custom setup network SSID / PSK or DSS host.
    userContext->setupNetworkSsid = NULL;
    userContext->setupNetworkPsk = NULL;
    userContext->dssHost = NULL;
    userContext->dssPort = 0;
    userContext->hasDssPort = false;

    // Define the DSS certificate paths.
    userContext->serverCaCertificatesPath = DSS_SERVER_CA_CERTIFICATES_PATH;
    userContext->clientCertificatePath = DSS_CLIENT_CERTIFICATE_PATH;
    userContext->clientCertificatePrivateKeyPath = DSS_CLIENT_CERTIFICATE_PRIVATE_KEY_PATH;
    userContext->cloudPublicKey = NULL; // This is loaded from ffs_linux_main function.

    // Define the device private key.
    userContext->devicePrivateKey = NULL;

    // Define the device public key.
    userContext->devicePublicKey = NULL;

    // Parse the private key file.
    FILE *privateKeyFile = fopen(DSS_CLIENT_CERTIFICATE_PRIVATE_KEY_PATH, "r");

    if (!privateKeyFile) {
        ffsLogDebug("Unable to read PrivateKey from file.");
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }
    userContext->devicePrivateKey = PEM_read_PrivateKey(privateKeyFile, NULL, NULL, NULL);
    fclose(privateKeyFile);

    ffsInitializeDevicePublicKey(userContext);

    //Success?
    if (!userContext->devicePrivateKey) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    // Allocate DSS buffers.
    userContext->hostNameBuffer = (uint8_t *)malloc(sizeof(uint8_t) * DSS_HOST_NAME_BUFFER_SIZE);
    if (!userContext->hostNameBuffer) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }
    userContext->sessionIdBuffer = (uint8_t *)malloc(sizeof(uint8_t) * DSS_SESSION_ID_BUFFER_SIZE);
    if (!userContext->sessionIdBuffer) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }
    userContext->nonceBuffer = (uint8_t *)malloc(sizeof(uint8_t) * DSS_NONCE_BUFFER_SIZE);
    if (!userContext->nonceBuffer) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }
    userContext->bodyBuffer = (uint8_t *)malloc(sizeof(uint8_t) * DSS_BODY_BUFFER_SIZE);
    if (!userContext->bodyBuffer) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Deinitialize the Ffs Wi-Fi Linux user context.
 */
FFS_RESULT ffsDeinitializeUserContext(FfsUserContext_t *userContext) {
    // Deinitialize the configuration map.
    if (ffsDeinitializeConfigurationMap(&userContext->configurationMap)) {
        ffsLogWarning("Failed to deinitialize configuration map");
    }

    // Deinitialize the Wi-Fi context.
    if (ffsDeinitializeWifiContext(&userContext->wifiContext)) {
        ffsLogWarning("Failed to deinitialzie Wi-Fi context");
    }

    // Destroy the provisionee state mutex.
    if (pthread_mutex_destroy(&userContext->provisioneeStateMutex)) {
        ffsLogWarning("Failed to destroy provisionee state mutex");
    }

    // Close semaphores.
    if (userContext->ffsTaskWifiSemaphore != SEM_FAILED) sem_close(userContext->ffsTaskWifiSemaphore);
    if (userContext->backgroundWifiScanSemaphore != SEM_FAILED) sem_close(userContext->backgroundWifiScanSemaphore);

    // Free command line arguments.
    if (userContext->setupNetworkSsid) free(userContext->setupNetworkSsid);
    if (userContext->setupNetworkPsk) free(userContext->setupNetworkPsk);
    if (userContext->dssHost) free(userContext->dssHost);

    // Free DSS buffers.
    if (userContext->hostNameBuffer) free(userContext->hostNameBuffer);
    if (userContext->sessionIdBuffer) free(userContext->sessionIdBuffer);
    if (userContext->nonceBuffer) free(userContext->nonceBuffer);
    if (userContext->bodyBuffer) free(userContext->bodyBuffer);

    // Free the EVP_PKEY structures.
    if (userContext->cloudPublicKey) EVP_PKEY_free(userContext->cloudPublicKey);
    if (userContext->devicePrivateKey) EVP_PKEY_free(userContext->devicePrivateKey);
    if (userContext->devicePublicKey) EVP_PKEY_free(userContext->devicePublicKey);

    return FFS_SUCCESS;
}

/*
 * Read the public key into user context. If the passed path is NULL,
 * we will use the default public key path.
 */
FFS_RESULT ffsInitializePublicKey(struct FfsUserContext_s *userContext, const char *cloudPublicKeyPath)
{
    if (!cloudPublicKeyPath) {
        ffsLogDebug("No custom public key path provided.");
        cloudPublicKeyPath = DPSS_PUBLIC_KEY_DEFAULT_PATH;
    }

    // Parse the file.
    FILE *publicKeyFile = fopen(cloudPublicKeyPath, "r");
    if (!publicKeyFile) {
        ffsLogDebug("Unable to read PublicKey from file.");
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }
    userContext->cloudPublicKey = PEM_read_PUBKEY(publicKeyFile, NULL, NULL, NULL);
    fclose(publicKeyFile);

    //Success?
    if (!userContext->cloudPublicKey) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Set a configuration value (\a e.g., country code).
 */
FFS_RESULT ffsSetConfigurationValue(struct FfsUserContext_s *userContext,
        const char *configurationKey, FfsMapValue_t *configurationValue)
{
    FFS_CHECK_RESULT(ffsSetConfigurationMapValue(userContext, configurationKey, configurationValue));
    return FFS_SUCCESS;
}

/*
 * Get a configuration value (\a e.g., country code).
 */
FFS_RESULT ffsGetConfigurationValue(struct FfsUserContext_s *userContext,
        const char *configurationKey, FfsMapValue_t *configurationValue)
{
    FFS_CHECK_RESULT(ffsGetConfigurationMapValue(userContext, configurationKey, configurationValue));
    return FFS_SUCCESS;
}

/*
 * Set the registration token (session ID).
 */
FFS_RESULT ffsSetRegistrationToken(struct FfsUserContext_s *userContext, FfsRegistrationRequest_t *registrationRequest)
{
    (void) userContext;

    ffsLogStream("Storing registration token:", &registrationRequest->tokenStream);

    return FFS_SUCCESS;
}

/*
 * Get the current registration status.
 */
FFS_RESULT ffsGetRegistrationDetails(struct FfsUserContext_s *userContext, FfsRegistrationDetails_t *registrationDetails)
{
    (void) userContext;

    registrationDetails->state = FFS_REGISTRATION_STATE_COMPLETE;

    return FFS_SUCCESS;
}

/*
 * Get the client-allocated buffers used by the Device Setup Service client.
 */
FFS_RESULT ffsDssClientGetBuffers(struct FfsUserContext_s *userContext,
        FfsStream_t *hostNameStream, FfsStream_t *sessionIdStream,
        FfsStream_t *nonceStream, FfsStream_t *bodyStream)
{
    if (!userContext->hostNameBuffer || !userContext->sessionIdBuffer
            || !userContext->nonceBuffer || !userContext->bodyBuffer) {
        ffsLogError("One or more DSS buffers not allocated");
        FFS_FAIL(FFS_ERROR);
    }

    *hostNameStream = ffsCreateOutputStream(userContext->hostNameBuffer, DSS_HOST_NAME_BUFFER_SIZE);
    *sessionIdStream = ffsCreateOutputStream(userContext->sessionIdBuffer, DSS_SESSION_ID_BUFFER_SIZE);
    *nonceStream = ffsCreateOutputStream(userContext->nonceBuffer, DSS_NONCE_BUFFER_SIZE);
    *bodyStream = ffsCreateOutputStream(userContext->bodyBuffer, DSS_BODY_BUFFER_SIZE);

    return FFS_SUCCESS;
}

/*
 * Set the state of the provisionee.
 */
FFS_RESULT ffsSetWifiProvisioneeState(struct FfsUserContext_s *userContext,
        FFS_WIFI_PROVISIONEE_STATE provisioneeState)
{
    if (pthread_mutex_lock(&userContext->provisioneeStateMutex)) {
        FFS_FAIL(FFS_ERROR);
    }

    userContext->provisioneeState = provisioneeState;

    if (pthread_mutex_unlock(&userContext->provisioneeStateMutex)) {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Get the state of the provisionee.
 */
FFS_RESULT ffsGetWifiProvisioneeState(struct FfsUserContext_s *userContext,
        FFS_WIFI_PROVISIONEE_STATE *provisioneeState)
{
    if (pthread_mutex_lock(&userContext->provisioneeStateMutex)) {
        FFS_FAIL(FFS_ERROR);
    }

    *provisioneeState = userContext->provisioneeState;

    if (pthread_mutex_unlock(&userContext->provisioneeStateMutex)) {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Gets the client's custom setup network configuration, if one exists.
 */
FFS_RESULT ffsGetSetupNetworkConfiguration(struct FfsUserContext_s *userContext,
        FfsWifiConfiguration_t *wifiConfiguration)
{
    if (userContext->setupNetworkSsid) {

        FFS_CHECK_RESULT(ffsFlushStream(&wifiConfiguration->ssidStream));
        FFS_CHECK_RESULT(ffsFlushStream(&wifiConfiguration->keyStream));

        FFS_CHECK_RESULT(ffsWriteStringToStream(userContext->setupNetworkSsid, &wifiConfiguration->ssidStream));

        if (userContext->setupNetworkPsk) {
            FFS_CHECK_RESULT(ffsWriteStringToStream(userContext->setupNetworkPsk, &wifiConfiguration->keyStream));
            wifiConfiguration->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
        } else {
            wifiConfiguration->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
        }

        return FFS_SUCCESS;
    }

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Let the client kill the Wi-Fi provisionee task with a boolean value.
 */
FFS_RESULT ffsWifiProvisioneeCanProceed(struct FfsUserContext_s *userContext,
        bool *canProceed)
{
    (void) userContext;

    *canProceed = true;

    return FFS_SUCCESS;
}

/*
 * Let the client control whether the Wi-Fi provisionee task continues to post Wi-Fi scan data.
 */
FFS_RESULT ffsWifiProvisioneeCanPostWifiScanData(struct FfsUserContext_s *userContext,
        uint32_t sequenceNumber, uint32_t totalCredentialsFound, bool allCredentialsFound, bool *canPostWifiScanData)
{
    // We have no practical limit on the number of credentials we can store.
    (void) totalCredentialsFound;
    (void) sequenceNumber;

    // Check that we have not found all credentials.
    if (allCredentialsFound) {
        *canPostWifiScanData = false;
        return FFS_SUCCESS;
    }

    // Check that we have not posted all of our networks.
    size_t scanListSize = 0;
    FFS_CHECK_RESULT(ffsWifiScanListGetSize(&userContext->wifiContext, &scanListSize));
    if (userContext->wifiContext.scanListIndex >= scanListSize) {
        *canPostWifiScanData = false;
        return FFS_SUCCESS;
    }

    *canPostWifiScanData = true;
    return FFS_SUCCESS;
}

/*
 * Let the client control whether the Wi-Fi provisionee task continues to get Wi-Fi credentials.
 */
FFS_RESULT ffsWifiProvisioneeCanGetWifiCredentials(struct FfsUserContext_s *userContext,
        uint32_t sequenceNumber, bool allCredentialsReturned, bool *canGetWifiCredentials)
{
    (void) userContext;
    (void) sequenceNumber;

    // Check that we have not returned all credentials.
    if (allCredentialsReturned) {
        *canGetWifiCredentials = false;
        return FFS_SUCCESS;
    }

    *canGetWifiCredentials = true;
    return FFS_SUCCESS;
}

/*
 * Extracts device public key from a file containing the device certificate or certificate chain.
 */
static FFS_RESULT ffsInitializeDevicePublicKey(struct FfsUserContext_s *userContext)
{
    // Try to open the certificate chain file.
    FILE *certChainFile = fopen(userContext->clientCertificatePath, "r");
    if (!certChainFile) {
        ffsLogDebug("Unable to read cert chain from file.:%s", userContext->clientCertificatePath);
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    // If you want to read the whole chain you can call this function multiple times.
    // Assumption is the first certificate in the chain is the leaf device certificate.
    X509 *clientCert = PEM_read_X509(certChainFile, NULL, NULL, NULL);
    if (!clientCert) {
        ffsLogError("Unable to read cert chain from file.:%s", userContext->clientCertificatePath);
        fclose(certChainFile);
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    fclose(certChainFile);

    // Now extract the public key from the cert.
    userContext->devicePublicKey = X509_get_pubkey(clientCert);

    //Success?
    if (!userContext->devicePublicKey) {
        FFS_CHECK_RESULT(ffsDeinitializeUserContext(userContext));
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
