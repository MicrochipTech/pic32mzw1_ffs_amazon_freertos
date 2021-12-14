/** @file ffs_wifi.c
 *
 * @brief Wi-Fi-related types.
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
#include "ffs/common/ffs_wifi.h"

/*
 * Translate a Ffs Wi-Fi security protocol to a readable string.
 */
FFS_RESULT ffsGetWifiSecurityProtocolString(FFS_WIFI_SECURITY_PROTOCOL securityProtocol,
        const char **securityProtocolString)
{
    switch (securityProtocol) {
        case FFS_WIFI_SECURITY_PROTOCOL_NONE:
            *securityProtocolString = "OPEN";
            return FFS_SUCCESS;
        case FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK:
            *securityProtocolString = "WPA_PSK";
            return FFS_SUCCESS;
        case FFS_WIFI_SECURITY_PROTOCOL_WEP:
            *securityProtocolString = "WEP";
            return FFS_SUCCESS;
        case FFS_WIFI_SECURITY_PROTOCOL_OTHER:
            *securityProtocolString = "OTHER";
            return FFS_SUCCESS;
        case FFS_WIFI_SECURITY_PROTOCOL_UNKNOWN:
            *securityProtocolString = "UNKNOWN";
            return FFS_SUCCESS;
        default:
            FFS_FAIL(FFS_ERROR);
    }
}

/*
 * Translate a Ffs Wi-Fi connection state to a readable string.
 */
FFS_RESULT ffsGetWifiConnectionStateString(FFS_WIFI_CONNECTION_STATE wifiConnectionState,
        const char **connectionStateString)
{
    switch (wifiConnectionState) {
        case FFS_WIFI_CONNECTION_STATE_IDLE:
            *connectionStateString = "IDLE";
            return FFS_SUCCESS;
        case FFS_WIFI_CONNECTION_STATE_DISCONNECTED:
            *connectionStateString = "DISCONNECTED";
            return FFS_SUCCESS;
        case FFS_WIFI_CONNECTION_STATE_UNAUTHENTICATED:
            *connectionStateString = "UNAUTHENTICATED";
            return FFS_SUCCESS;
        case FFS_WIFI_CONNECTION_STATE_AUTHENTICATED:
            *connectionStateString = "AUTHENTICATED";
            return FFS_SUCCESS;
        case FFS_WIFI_CONNECTION_STATE_ASSOCIATED:
            *connectionStateString = "ASSOCIATED";
            return FFS_SUCCESS;
        case FFS_WIFI_CONNECTION_STATE_FAILED:
            *connectionStateString = "FAILED";
            return FFS_SUCCESS;
        default:
            FFS_FAIL(FFS_ERROR);
    }
}

