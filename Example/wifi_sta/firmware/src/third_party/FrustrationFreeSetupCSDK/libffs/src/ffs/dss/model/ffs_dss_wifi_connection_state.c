/** @file ffs_dss_wifi_connection_state.c
 *
 * @brief DSS Wi-Fi connection state enumeration implementation.
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
#include "ffs/dss/model/ffs_dss_wifi_connection_state.h"

#define CONNECTION_STATE_IDLE_STRING                "IDLE"
#define CONNECTION_STATE_DISCONNECTED_STRING        "DISCONNECTED"
#define CONNECTION_STATE_UNAUTHENTICATED_STRING     "UNAUTHENTICATED"
#define CONNECTION_STATE_AUTHENTICATED_STRING       "AUTHENTICATED"
#define CONNECTION_STATE_ASSOCIATED_STRING          "ASSOCIATED"
#define CONNECTION_STATE_FAILED_STRING              "FAILED"

/*
 * Translate a DSS Wi-Fi connection state to the DSS API model string.
 */
FFS_RESULT ffsDssGetWifiConnectionStateString(FFS_DSS_WIFI_CONNECTION_STATE connectionState,
        const char **connectionStateString)
{
    switch (connectionState) {
        case FFS_DSS_WIFI_CONNECTION_STATE_IDLE:
            *connectionStateString = CONNECTION_STATE_IDLE_STRING;
            return FFS_SUCCESS;
        case FFS_DSS_WIFI_CONNECTION_STATE_DISCONNECTED:
            *connectionStateString = CONNECTION_STATE_DISCONNECTED_STRING;
            return FFS_SUCCESS;
        case FFS_DSS_WIFI_CONNECTION_STATE_UNAUTHENTICATED:
            *connectionStateString = CONNECTION_STATE_UNAUTHENTICATED_STRING;
            return FFS_SUCCESS;
        case FFS_DSS_WIFI_CONNECTION_STATE_AUTHENTICATED:
            *connectionStateString = CONNECTION_STATE_AUTHENTICATED_STRING;
            return FFS_SUCCESS;
        case FFS_DSS_WIFI_CONNECTION_STATE_ASSOCIATED:
            *connectionStateString = CONNECTION_STATE_ASSOCIATED_STRING;
            return FFS_SUCCESS;
        case FFS_DSS_WIFI_CONNECTION_STATE_FAILED:
            *connectionStateString = CONNECTION_STATE_FAILED_STRING;
            return FFS_SUCCESS;
        default:
            FFS_FAIL(FFS_ERROR);
    }
}

/*
 * Parse a DSS model API string to a Ffs Wi-Fi connection state.
 */
FFS_RESULT ffsDssParseWifiConnectionState(const char *connectionStateString,
        FFS_DSS_WIFI_CONNECTION_STATE *connectionState)
{
    if (!strcmp(CONNECTION_STATE_IDLE_STRING, connectionStateString)) {
        *connectionState = FFS_DSS_WIFI_CONNECTION_STATE_IDLE;
    } else if (!strcmp(CONNECTION_STATE_DISCONNECTED_STRING, connectionStateString)) {
        *connectionState = FFS_DSS_WIFI_CONNECTION_STATE_DISCONNECTED;
    } else if (!strcmp(CONNECTION_STATE_UNAUTHENTICATED_STRING, connectionStateString)) {
        *connectionState = FFS_DSS_WIFI_CONNECTION_STATE_UNAUTHENTICATED;
    } else if (!strcmp(CONNECTION_STATE_AUTHENTICATED_STRING, connectionStateString)) {
        *connectionState = FFS_DSS_WIFI_CONNECTION_STATE_AUTHENTICATED;
    } else if (!strcmp(CONNECTION_STATE_ASSOCIATED_STRING, connectionStateString)) {
        *connectionState = FFS_DSS_WIFI_CONNECTION_STATE_ASSOCIATED;
    } else if (!strcmp(CONNECTION_STATE_FAILED_STRING, connectionStateString)) {
        *connectionState = FFS_DSS_WIFI_CONNECTION_STATE_FAILED;
    } else {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
