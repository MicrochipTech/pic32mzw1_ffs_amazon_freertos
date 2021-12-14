/** @file ffs_amazon_freertos_wifi_compat.c
 *
 * @brief FFS RTOS wifi compat functions.
 *
 * @copyright 2020 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "FreeRTOS.h"
#include "iot_wifi.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_user_context.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_directed_scan.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_wifi_manager.h"
#include "ffs/amazon_freertos/ffs_amazon_freertos_https_client.h"
#include "ffs/common/ffs_stream.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"
#include "ffs/common/ffs_check_result.h"

#define WIFI_MAC_ADDRESS_LENGTH             (6)

static WIFIScanResult_t sWifiScanResultItem;

static FFS_RESULT ffsGetWifiConnectionErrorDetails(const FFS_WIFI_CONNECTION_STATE attemptState, bool *const hasDetails, FfsErrorDetails_t *const errorDetails);

/*
 * Get the next Wi-Fi scan result.
 */
FFS_RESULT ffsGetWifiScanResult(FfsUserContext_t *userContext, FfsWifiScanResult_t *wifiScanResult, bool *isUnderrun)
{
    // Get and increment scan list index.
    const uint8_t scanListIndex = userContext->scanListIndex++;

    // Do we still have scan results?
    uint8_t numAp;
    ffsWifiManagerGetScannedNumberOfAps(userContext, &numAp);
    *isUnderrun = scanListIndex >= numAp;
    if (*isUnderrun) {
        FFS_CHECK_RESULT(ffsWifiManagerResetScanResults(userContext));
        return FFS_SUCCESS;
    }

    // Get the next scan result.
    FFS_CHECK_RESULT(ffsWifiManagerGetScanResult(userContext, &sWifiScanResultItem, scanListIndex));

    // Convert to an FFS scan result.
    wifiScanResult->ssidStream = ffsCreateInputStream((uint8_t*) sWifiScanResultItem.cSSID, strlen(sWifiScanResultItem.cSSID));
    wifiScanResult->bssidStream = ffsCreateInputStream(sWifiScanResultItem.ucBSSID, WIFI_MAC_ADDRESS_LENGTH);
    wifiScanResult->frequencyBand = sWifiScanResultItem.cChannel;
    wifiScanResult->signalStrength = sWifiScanResultItem.cRSSI;

    // Convert key management.
    switch (sWifiScanResultItem.xSecurity) 
    {
    case eWiFiSecurityOpen:
        /* 
         * Getting a cipher of a network is unavailable in iot_wifi.h
         * Assume all WIFI_AUTH_OPEN have no security.
         */
        wifiScanResult->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
        break;
    case eWiFiSecurityWPA:
        // Fall-through.
    case eWiFiSecurityWPA2:
        wifiScanResult->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
        break;
    default:
        wifiScanResult->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_OTHER;
        break;
    }

    return FFS_SUCCESS;
}

/*
 * Provide Wi-Fi network credentials to the client.
 */
FFS_RESULT ffsAddWifiConfiguration(struct FfsUserContext_s *userContext, FfsWifiConfiguration_t *wifiConfiguration)
{
    (void) userContext;

    // Do we already have a network configured?
    if (userContext->hasWifiConfiguration)
    {
        ffsLogWarning("Wi-Fi network already configured - swallowing configuration: %.*s",
                FFS_STREAM_DATA_SIZE(wifiConfiguration->ssidStream), FFS_STREAM_NEXT_READ(wifiConfiguration->ssidStream));
        return FFS_SUCCESS;
    }

    ffsLogDebug("Add Wi-Fi configuration: %.*s", FFS_STREAM_DATA_SIZE(wifiConfiguration->ssidStream),
            FFS_STREAM_NEXT_READ(wifiConfiguration->ssidStream));
    
    // Convert to FreeRTOS configuration
    WIFINetworkProfile_t wifiCredentials;
    /**
     * FfsStream does not have '\0' ending.
     * Following line assures the converted string followed by a 0. 
     */
    memset(&wifiCredentials, 0, sizeof(wifiCredentials)); 
    // Copy SSID
    wifiCredentials.ucSSIDLength = FFS_STREAM_DATA_SIZE(wifiConfiguration->ssidStream);
    if (wifiCredentials.ucSSIDLength > FFS_MAXIMUM_SSID_SIZE) {
        FFS_FAIL(FFS_OVERRUN);
    }
    memcpy(wifiCredentials.cSSID, FFS_STREAM_NEXT_READ(wifiConfiguration->ssidStream),
            wifiCredentials.ucSSIDLength);
    // Copy key.
    wifiCredentials.ucPasswordLength = FFS_STREAM_DATA_SIZE(wifiConfiguration->keyStream);
    memcpy(wifiCredentials.cPassword, FFS_STREAM_NEXT_READ(wifiConfiguration->keyStream),
            wifiCredentials.ucPasswordLength);
    // Copy Security
    switch (wifiConfiguration->securityProtocol)
    {
    case FFS_WIFI_SECURITY_PROTOCOL_NONE:
        wifiCredentials.xSecurity = eWiFiSecurityOpen;
        break;
    case FFS_WIFI_SECURITY_PROTOCOL_WEP:
        wifiCredentials.xSecurity = eWiFiSecurityWEP;
        break;
    case FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK:
        wifiCredentials.xSecurity = eWiFiSecurityWPA2;
        break;
    default:
        ffsLogError("FFS_WIFI_SECURITY_PROTOCOL %d not supported.", wifiConfiguration->securityProtocol);
        wifiCredentials.xSecurity = eWiFiSecurityNotSupported;
    }

    // Load credentials.
    ffsWifiManagerLoadStaCredentials(userContext, &wifiCredentials);
    return FFS_SUCCESS;
}

FFS_RESULT ffsRemoveWifiConfiguration(struct FfsUserContext_s *userContext, FfsStream_t ssidStream) 
{
    (void) userContext;
    (void) ssidStream;

    WIFINetworkProfile_t currentConfiguration;
    FFS_CHECK_RESULT(ffsWifiManagerGetStaCredentials(userContext, &currentConfiguration));
    if (ffsStreamMatchesString(&ssidStream, currentConfiguration.cSSID))
    {
        userContext->hasWifiConfiguration = false;
        FFS_CHECK_RESULT(ffsWifiManagerClearStaCredentials(userContext));
    }

    return FFS_SUCCESS;
}

/*
 * Start connecting to the stored Wi-Fi network(s).
 */
FFS_RESULT ffsConnectToWifi(struct FfsUserContext_s *userContext)
{
    FFS_CHECK_RESULT(ffsWifiManagerActiveScanForNetwork(userContext));
    FFS_CHECK_RESULT(ffsWifiManagerConnect(userContext));

    return FFS_SUCCESS;
}

/*
 * Get the current Wi-Fi connection state.
 */
FFS_RESULT ffsGetWifiConnectionDetails(struct FfsUserContext_s *userContext, FfsWifiConnectionDetails_t *wifiConnectionDetails)
{
    (void) userContext;

    WIFINetworkProfile_t wifiNetworkProfile;
    FFS_WIFI_CONNECTION_STATE connectionState;
    FFS_CHECK_RESULT(ffsWifiManagerGetConnectionDetails(userContext, &wifiNetworkProfile, &connectionState));

    wifiConnectionDetails->ssidStream = ffsCreateInputStream((uint8_t*)wifiNetworkProfile.cSSID, wifiNetworkProfile.ucSSIDLength);
    switch(wifiNetworkProfile.xSecurity)
    {
    case eWiFiSecurityOpen:
        wifiConnectionDetails->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
        break;
    case eWiFiSecurityWPA:
    case eWiFiSecurityWPA2:
    case eWiFiSecurityWPA2_ent:
        wifiConnectionDetails->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
        break;
    default:
        ffsLogWarning("Unsupported security mode");
        wifiConnectionDetails->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_UNKNOWN;
    }

    // Set the state.
    wifiConnectionDetails->state = connectionState;

    // Use default error details if needed.
    wifiConnectionDetails->hasErrorDetails = false;
    
    ffsLogStream("Returned connection details for network:", &wifiConnectionDetails->ssidStream);

    return FFS_SUCCESS;
}

FFS_RESULT ffsGetWifiConnectionAttempt(struct FfsUserContext_s *userContext, FfsWifiConnectionAttempt_t *wifiConnectionAttempt,
        bool *isUnderrun) 
{
    (void) userContext;

    // Get attempted AP profile and its connection result from Wifi Manager.
    WIFINetworkProfile_t wifiNetworkProfile;
    const uint8_t attemptListIndex = userContext->attemptListIndex++;
    FFS_WIFI_CONNECTION_STATE attemptState;
    FFS_CHECK_RESULT(ffsWifiManagerGetConnectionAttempt(userContext, attemptListIndex, &wifiNetworkProfile, &attemptState, isUnderrun));
    if (*isUnderrun)
    {
        userContext->attemptListIndex--;
        return FFS_SUCCESS;
    }

    // Covert it into FfsWifiConnectionAttempt_t
    wifiConnectionAttempt->ssidStream = ffsCreateInputStream((uint8_t*)wifiNetworkProfile.cSSID, wifiNetworkProfile.ucSSIDLength);
    switch(wifiNetworkProfile.xSecurity)
    {
    case eWiFiSecurityOpen:
        wifiConnectionAttempt->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
        break;
    case eWiFiSecurityWPA:
    case eWiFiSecurityWPA2:
    case eWiFiSecurityWPA2_ent:
        wifiConnectionAttempt->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
        break;
    default:
        ffsLogWarning("Unsupported security mode");
        wifiConnectionAttempt->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_UNKNOWN;
    }

    ffsGetWifiConnectionErrorDetails(attemptState, &wifiConnectionAttempt->hasErrorDetails, &wifiConnectionAttempt->errorDetails);

    ffsLogStream("Returned connection attempt:", &wifiConnectionAttempt->ssidStream);

    return FFS_SUCCESS;
}

/*
 * Let the client control whether the Wi-Fi provisionee task continues to post Wi-Fi scan data.
 */
FFS_RESULT ffsWifiProvisioneeCanPostWifiScanData(struct FfsUserContext_s *userContext, 
        uint32_t sequenceNumber, uint32_t totalCredentialsFound, bool allCredentialsFound,
        bool *canPostWifiScanData)
{
    // New sequence?
    if (sequenceNumber == 1) {
        userContext->scanListIndex = 0;
    }

    // Any credentials found?
    if (totalCredentialsFound) {
        *canPostWifiScanData = false;
        return FFS_SUCCESS;
    }

    // Get the Wi-Fi scan list.
    uint8_t numAp;
    FFS_CHECK_RESULT(ffsWifiManagerGetScannedNumberOfAps(userContext, &numAp));

    // Do we still have scan results?
    *canPostWifiScanData = userContext->scanListIndex < numAp;

    return FFS_SUCCESS;
}

FFS_RESULT ffsWifiProvisioneeCanGetWifiCredentials(struct FfsUserContext_s *userContext, uint32_t sequenceNumber,
        bool allCredentialsReturned, bool *canGetWifiCredentials)
{
    // New sequence?
    if (sequenceNumber == 1) {
        userContext->hasWifiConfiguration = false;
    }

    // All credentials returned?
    if (allCredentialsReturned) {
        *canGetWifiCredentials = false;
        return FFS_SUCCESS;
    }

    // Do we already have a Wi-Fi configuration?
    if (!userContext->hasWifiConfiguration) {
        *canGetWifiCredentials = true;
        return FFS_SUCCESS;
    }

    // Get credentials.
    *canGetWifiCredentials = true;

    return FFS_SUCCESS;
}

FFS_RESULT ffsSetWifiProvisioneeState(struct FfsUserContext_s *userContext,
        FFS_WIFI_PROVISIONEE_STATE provisioneeState)
{
    // Used in BT layer - NULL checks in case FFS isn't running.
    if (!userContext) {
        FFS_FAIL(FFS_ERROR);
    }

    // Set the state on the user context.
    userContext->state = provisioneeState;

    return FFS_SUCCESS;
}

FFS_RESULT ffsGetWifiProvisioneeState(struct FfsUserContext_s *userContext,
        FFS_WIFI_PROVISIONEE_STATE *provisioneeState)
{
    // Used in BT layer - NULL checks in case FFS isn't running.
    if (!userContext) {
        FFS_FAIL(FFS_ERROR);
    }

    *provisioneeState = userContext->state;
    return FFS_SUCCESS;
}

FFS_RESULT ffsGetSetupNetworkConfiguration(struct FfsUserContext_s *userContext,
        FfsWifiConfiguration_t *wifiConfiguration)
{
    return FFS_NOT_IMPLEMENTED;
}

FFS_RESULT ffsWifiProvisioneeCanProceed(struct FfsUserContext_s *userContext,
        bool *canProceed)
{
    *canProceed = true;
    return FFS_SUCCESS;
}

static FFS_RESULT ffsGetWifiConnectionErrorDetails(const FFS_WIFI_CONNECTION_STATE attemptState, bool *const hasErrorDetails, FfsErrorDetails_t *const errorDetails)
{
    // The only failure wifi manager gives.
    if(attemptState == FFS_WIFI_CONNECTION_STATE_FAILED)
    {
        *hasErrorDetails = true;
        errorDetails->operation = "CONNECTING_TO_NETWORK";
        errorDetails->cause = "Internal error";
        errorDetails->details = "Internal error";
        errorDetails->code = "3:3:0:1";
    }
    else
    {
        *hasErrorDetails = false;
    }
    return FFS_SUCCESS;
}