/** @file ffs_linux_wifi.c
 *
 * @brief Linux compat-layer Wi-Fi function implementations.
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
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/linux/ffs_wifi_connection_attempt_list.h"
#include "ffs/linux/ffs_wifi_configuration_list.h"
#include "ffs/linux/ffs_wifi_manager.h"
#include "ffs/linux/ffs_wifi_scan_list.h"

#define FFS_WIFI_WPA_SUPPLICANT_CONFIGURATION_FILE ("Ffs.conf")

static void ffsWifiManagerOperationCallback(struct FfsUserContext_s *userContext, FFS_RESULT result);

/*
 * Get a Wi-Fi scan result.
 */
FFS_RESULT ffsGetWifiScanResult(struct FfsUserContext_s *userContext, FfsWifiScanResult_t *wifiScanResult, bool *isUnderrun)
{

    FfsLinuxWifiContext_t *wifiContext = &userContext->wifiContext;

    size_t scanListSize = 0;
    FFS_CHECK_RESULT(ffsWifiScanListGetSize(wifiContext, &scanListSize));
    if (wifiContext->scanListIndex >= scanListSize) {
        *isUnderrun = true;
        return FFS_SUCCESS;
    }

    // Peek the next scan result.
    FfsWifiScanResult_t *nextWifiScanResult;
    FFS_CHECK_RESULT(ffsWifiScanListPeekIndex(wifiContext, wifiContext->scanListIndex, &nextWifiScanResult));
    ++wifiContext->scanListIndex;

    // Copy the stored scan result.
    FFS_CHECK_RESULT(ffsAppendStream(&nextWifiScanResult->ssidStream, &wifiScanResult->ssidStream));
    FFS_CHECK_RESULT(ffsAppendStream(&nextWifiScanResult->bssidStream, &wifiScanResult->bssidStream));
    wifiScanResult->securityProtocol = nextWifiScanResult->securityProtocol;
    wifiScanResult->frequencyBand = nextWifiScanResult->frequencyBand;
    wifiScanResult->signalStrength = nextWifiScanResult->signalStrength;

    // Rewind stored scan result streams.
    FFS_CHECK_RESULT(ffsRewindStream(&nextWifiScanResult->ssidStream));
    FFS_CHECK_RESULT(ffsRewindStream(&nextWifiScanResult->bssidStream));

    return FFS_SUCCESS;
}

/*
 * Provide Wi-Fi network credentials to the client.
 */
FFS_RESULT ffsAddWifiConfiguration(struct FfsUserContext_s *userContext, FfsWifiConfiguration_t *wifiConfiguration)
{
    FFS_CHECK_RESULT(ffsWifiConfigurationListPush(&userContext->wifiContext, wifiConfiguration));
    return FFS_SUCCESS;
}

/*
 * Remove a stored Wi-Fi configuration.
 */
FFS_RESULT ffsRemoveWifiConfiguration(struct FfsUserContext_s *userContext, FfsStream_t ssidStream)
{

    FfsLinuxWifiContext_t *wifiContext = &userContext->wifiContext;

    ffsLogDebug("Remove configurations with SSID '%.*s'", FFS_STREAM_DATA_SIZE(ssidStream),
            (const char *)FFS_STREAM_NEXT_READ(ssidStream));

    // Are we currently connected to this SSID?
    bool matchesConnectionDetails = ffsStreamMatchesStream(&wifiContext->connectionDetails.ssidStream, &ssidStream);

    // If yes, disconnect.
    if (matchesConnectionDetails) {

        FFS_CHECK_RESULT(ffsWifiManagerDisconnect(ffsWifiManagerOperationCallback));

        // Wait for the disconnection to complete.
        if (sem_wait(userContext->ffsTaskWifiSemaphore)) {
            ffsLogError("Failed to wait on network disconnection semaphore");
            FFS_FAIL(FFS_ERROR);
        }
    }

    // Remove the configuration.
    FFS_CHECK_RESULT(ffsWifiConfigurationListPopConfiguration(wifiContext, ssidStream));

    return FFS_SUCCESS;
}

/*
 * Start connecting to the stored Wi-Fi network(s).
 */
FFS_RESULT ffsConnectToWifi(struct FfsUserContext_s *userContext)
{

    // Get the DSS host.
    FFS_TEMPORARY_OUTPUT_STREAM(dssHostStream, 128);
    uint16_t port;
    FFS_CHECK_RESULT(ffsDssClientGetDefaultHost(userContext, &dssHostStream, &port));

    // Start the connection attempts.
    FFS_CHECK_RESULT(ffsWifiManagerConnect(FFS_WIFI_WPA_SUPPLICANT_CONFIGURATION_FILE, &dssHostStream,
            ffsWifiManagerOperationCallback));

    // Wait for the connection attempt to complete.
    if (sem_wait(userContext->ffsTaskWifiSemaphore)) {
        ffsLogError("Failed to wait on network connection semaphore");
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Get the current Wi-Fi connection state.
 */
FFS_RESULT ffsGetWifiConnectionDetails(struct FfsUserContext_s *userContext,
        FfsWifiConnectionDetails_t *wifiConnectionDetails)
{

    FfsLinuxWifiContext_t *wifiContext = &userContext->wifiContext;

    // Copy the connection details.
    FfsStream_t ssidStream = wifiContext->connectionDetails.ssidStream;
    FFS_CHECK_RESULT(ffsAppendStream(&ssidStream, &wifiConnectionDetails->ssidStream));
    wifiConnectionDetails->securityProtocol = wifiContext->connectionDetails.securityProtocol;
    wifiConnectionDetails->state = wifiContext->connectionDetails.state;
    wifiConnectionDetails->hasErrorDetails = wifiContext->connectionDetails.hasErrorDetails;
    wifiConnectionDetails->errorDetails = wifiContext->connectionDetails.errorDetails;

    return FFS_SUCCESS;
}

/*
 * Get the next Wi-Fi connection attempt.
 */
FFS_RESULT ffsGetWifiConnectionAttempt(struct FfsUserContext_s *userContext,
        FfsWifiConnectionAttempt_t *wifiConnectionAttempt, bool *isUnderrun)
{

    bool connectionAttemptListIsEmpty = true;
    FFS_CHECK_RESULT(ffsWifiConnectionAttemptListIsEmpty(&userContext->wifiContext, &connectionAttemptListIsEmpty));
    if (connectionAttemptListIsEmpty) {
        *isUnderrun = true;
        return FFS_SUCCESS;
    }

    // Peek the next scan result.
    FfsWifiConnectionAttempt_t *nextWifiConnectionAttempt;
    FFS_CHECK_RESULT(ffsWifiConnectionAttemptListPeek(&userContext->wifiContext, &nextWifiConnectionAttempt));

    // Copy the stored scan result.
    FFS_CHECK_RESULT(ffsAppendStream(&nextWifiConnectionAttempt->ssidStream, &wifiConnectionAttempt->ssidStream));
    wifiConnectionAttempt->securityProtocol = nextWifiConnectionAttempt->securityProtocol;
    wifiConnectionAttempt->state = nextWifiConnectionAttempt->state;
    wifiConnectionAttempt->hasErrorDetails = nextWifiConnectionAttempt->hasErrorDetails;
    wifiConnectionAttempt->errorDetails = nextWifiConnectionAttempt->errorDetails;

    // Pop the stored scan result.
    FFS_CHECK_RESULT(ffsWifiConnectionAttemptListPop(&userContext->wifiContext));

    return FFS_SUCCESS;
}

static void ffsWifiManagerOperationCallback(struct FfsUserContext_s *userContext, FFS_RESULT result)
{
    ffsLogDebug("Wi-Fi manager operation completed with result %s", ffsGetResultString(result));

    if (sem_post(userContext->ffsTaskWifiSemaphore)) {
        ffsLogError("Unable to post to Wi-Fi operation semaphore");
    }
}
