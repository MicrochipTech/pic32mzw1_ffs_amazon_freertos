/** @file ffs_wifi_context.c
 *
 * @brief Ffs Linux context API
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
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"
#include "ffs/linux/ffs_linux_error_details.h"
#include "ffs/linux/ffs_wifi_configuration_list.h"
#include "ffs/linux/ffs_wifi_connection_attempt_list.h"
#include "ffs/linux/ffs_wifi_context.h"
#include "ffs/linux/ffs_wifi_scan_list.h"

#define FFS_WIFI_INTERFACE ("wlan0")
#define FFS_WIFI_DRIVER    ("wext")

/*
 * Initialize a Ffs Wi-Fi context.
 */
FFS_RESULT ffsInitializeWifiContext(FfsLinuxWifiContext_t *wifiContext)
{
    // Initialize the linked lists.
    FFS_CHECK_RESULT(ffsLinkedListInitialize(&wifiContext->configurationList));
    FFS_CHECK_RESULT(ffsLinkedListInitialize(&wifiContext->scanList));
    FFS_CHECK_RESULT(ffsLinkedListInitialize(&wifiContext->connectionAttemptList));

    // Create the scan list mutex.
    if (pthread_mutex_init(&wifiContext->scanListMutex, NULL)) {
        FFS_FAIL(FFS_ERROR);
    }

    wifiContext->lastBackgroundScanTime = 0;
    wifiContext->scanListIndex = 0;

    wifiContext->interface = FFS_WIFI_INTERFACE;
    wifiContext->driver = FFS_WIFI_DRIVER;
    wifiContext->connectionDetails.state = FFS_WIFI_CONNECTION_STATE_DISCONNECTED;
    wifiContext->wifiManagerInitialized = false;

    wifiContext->connectionDetails.ssidStream = ffsCreateOutputStream(wifiContext->ssidBuffer,
            FFS_MAXIMUM_SSID_SIZE);

    return FFS_SUCCESS;
}

/*
 * Deinitialize a Ffs Wi-Fi context.
 */
FFS_RESULT ffsDeinitializeWifiContext(FfsLinuxWifiContext_t *wifiContext)
{
    // Deinitialize the linked lists.
    FFS_CHECK_RESULT(ffsWifiConfigurationListClear(wifiContext));
    FFS_CHECK_RESULT(ffsWifiScanListClear(wifiContext));
    FFS_CHECK_RESULT(ffsWifiConnectionAttemptListClear(wifiContext));

    // Destroy the scan list mutex.
    if (pthread_mutex_destroy(&wifiContext->scanListMutex)) {
        ffsLogWarning("Failed to destroy Wi-Fi scan list mutex");
    }

    return FFS_SUCCESS;
}

/*
 * Update the Wi-Fi connection details.
 */
FFS_RESULT ffsUpdateWifiConnectionDetails(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration)
{
    FfsWifiConnectionDetails_t *connectionDetails = &wifiContext->connectionDetails;
    FfsStream_t ssidStream = configuration->ssidStream;

    FFS_CHECK_RESULT(ffsFlushStream(&connectionDetails->ssidStream));
    FFS_CHECK_RESULT(ffsAppendStream(&ssidStream, &connectionDetails->ssidStream));
    connectionDetails->securityProtocol = configuration->securityProtocol;

    ffsLogStream("Wi-Fi connection details updated:", &connectionDetails->ssidStream);
    return FFS_SUCCESS;
}

/*
 * Update the Wi-Fi connection state.
 */
FFS_RESULT ffsUpdateWifiConnectionState(FfsLinuxWifiContext_t *wifiContext,
        FFS_WIFI_CONNECTION_STATE state)
{
    ffsLogDebug("Wi-Fi connection state updated: %d", state);
    FfsWifiConnectionDetails_t *connectionDetails = &wifiContext->connectionDetails;

    if (state == FFS_WIFI_CONNECTION_STATE_FAILED) {
        ffsLogError("Use 'ffsUpdateWifiConnectionFailure' API in the failure case");
        FFS_FAIL(FFS_ERROR);
    }

    connectionDetails->state = state;
    connectionDetails->errorDetails = ffsErrorDetailsNull;

    if (state == FFS_WIFI_CONNECTION_STATE_IDLE || state == FFS_WIFI_CONNECTION_STATE_DISCONNECTED) {
        FFS_CHECK_RESULT(ffsFlushStream(&connectionDetails->ssidStream));
        connectionDetails->securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_UNKNOWN;
    }

    return FFS_SUCCESS;
}

/*
 * Update the Wi-Fi connection state in the failure case.
 */
FFS_RESULT ffsUpdateWifiConnectionFailure(FfsLinuxWifiContext_t *wifiContext,
        const FfsErrorDetails_t *errorDetails)
{
    FfsWifiConnectionDetails_t *connectionDetails = &wifiContext->connectionDetails;

    connectionDetails->state = FFS_WIFI_CONNECTION_STATE_FAILED;
    connectionDetails->errorDetails = *errorDetails;

    if (errorDetails) {
        ffsLogError("Wi-Fi connection failure");
        ffsLogError("Operation: %s", errorDetails->operation ? errorDetails->operation : "NULL");
        ffsLogError("Cause:     %s", errorDetails->cause ? errorDetails->cause : "NULL");
        ffsLogError("Details:   %s", errorDetails->details ? errorDetails->details : "NULL");
        ffsLogError("Code:      %s", errorDetails->code ? errorDetails->code : "NULL");
    } else {
        ffsLogError("Wi-Fi connection failure; no error details provided");
    }

    return FFS_SUCCESS;
}
