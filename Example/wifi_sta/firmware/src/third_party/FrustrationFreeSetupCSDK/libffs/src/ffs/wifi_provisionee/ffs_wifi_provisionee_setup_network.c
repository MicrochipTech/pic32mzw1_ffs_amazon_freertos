/** @file ffs_wifi_provisionee_setup_network.c
 *
 * @brief Wi-Fi FFS setup network implementation.
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "ffs/common/ffs_check_result.h"
#include "ffs/common/ffs_logging.h"
#include "ffs/compat/ffs_common_compat.h"
#include "ffs/compat/ffs_wifi_provisionee_compat.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_setup_network.h"

#define FFS_WIFI_SETUP_NETWORK_SSID                     "simple_setup"
#define FFS_WIFI_SETUP_NETWORK_IS_HIDDEN_NETWORK        true
#define FFS_WIFI_SETUP_NETWORK_WIFI_SECURITY_PROTOCOL   FFS_WIFI_SECURITY_PROTOCOL_NONE

/*
 *  Connect to the Wi-Fi FFS setup network.
 */
FFS_RESULT ffsConnectToSetupNetwork(struct FfsUserContext_s *userContext, FfsWifiConfiguration_t *setupWifiConfiguration)
{
    if (!setupWifiConfiguration
            || ffsStreamIsEmpty(&setupWifiConfiguration->ssidStream)) {
        ffsLogError("SSID not found for network.");
        return FFS_ERROR;
    }

    if (setupWifiConfiguration->securityProtocol != FFS_WIFI_SECURITY_PROTOCOL_NONE
            && ffsStreamIsEmpty(&setupWifiConfiguration->keyStream)) {
        ffsLogError("Key not found for network.");
        return FFS_ERROR;
    }

    FFS_TEMPORARY_OUTPUT_STREAM(ssidStream, FFS_MAXIMUM_SSID_SIZE);
    FfsWifiConnectionDetails_t connectionDetails = {
        .ssidStream = ssidStream
    };

    // Store the setup network configuration.
    FFS_CHECK_RESULT(ffsAddWifiConfiguration(userContext, setupWifiConfiguration));

    // Start the connection attempt.
    FFS_CHECK_RESULT(ffsConnectToWifi(userContext));

    // Get the connection state.
    FFS_CHECK_RESULT(ffsGetWifiConnectionDetails(userContext, &connectionDetails));

    // Fail if we are not connected to Wi-Fi.
    if (connectionDetails.state != FFS_WIFI_CONNECTION_STATE_ASSOCIATED) {
        ffsLogError("Failed to connect to setup network with final state %d", connectionDetails.state);
        FFS_FAIL(FFS_ERROR);
    }
    
    ffsLogDebug("Connected to setup network");

    return FFS_SUCCESS;
}

/*
 *  Disconnect from the Wi-Fi FFS setup network.
 */
FFS_RESULT ffsDisconnectFromSetupNetwork(struct FfsUserContext_s *userContext, FfsWifiConfiguration_t *setupWifiConfiguration)
{
    ffsLogDebug("Start disconnecting from setup network");
    if (!setupWifiConfiguration
            || ffsStreamIsEmpty(&setupWifiConfiguration->ssidStream)) {
        ffsLogError("SSID not found for encoded network.");
        FFS_FAIL(FFS_NOT_IMPLEMENTED);
    }

    // Remove the SSID. This function disconnects as well.
    FFS_CHECK_RESULT(ffsRemoveWifiConfiguration(userContext, setupWifiConfiguration->ssidStream));
    ffsLogDebug("Disconnected from setup network");
    return FFS_SUCCESS;
}

/*
 *  Get the default Wi-Fi FFS setup network configuration.
 */
FFS_RESULT ffsGetDefaultSetupNetworkConfiguration(FfsWifiConfiguration_t *wifiConfiguration)
{
    FFS_CHECK_RESULT(ffsFlushStream(&wifiConfiguration->ssidStream));
    FFS_CHECK_RESULT(ffsWriteStringToStream(FFS_WIFI_SETUP_NETWORK_SSID, &wifiConfiguration->ssidStream));
    FFS_CHECK_RESULT(ffsFlushStream(&wifiConfiguration->keyStream));
    wifiConfiguration->securityProtocol = FFS_WIFI_SETUP_NETWORK_WIFI_SECURITY_PROTOCOL;
    wifiConfiguration->isHiddenNetwork = FFS_WIFI_SETUP_NETWORK_IS_HIDDEN_NETWORK;

    return FFS_SUCCESS;
}

FFS_RESULT ffsGetFallbackSetupNetwork(struct FfsUserContext_s *userContext, FfsWifiConfiguration_t *wifiConfiguration)
{
    FFS_RESULT result = ffsGetSetupNetworkConfiguration(userContext, wifiConfiguration);
    if (result == FFS_NOT_IMPLEMENTED) {
        ffsLogDebug("Using our own default setup network");
        FFS_CHECK_RESULT(ffsGetDefaultSetupNetworkConfiguration(wifiConfiguration));
    } else {
        ffsLogDebug("Using client-defined setup network configuration");
        FFS_CHECK_RESULT(result);
    }

    return FFS_SUCCESS;
}

