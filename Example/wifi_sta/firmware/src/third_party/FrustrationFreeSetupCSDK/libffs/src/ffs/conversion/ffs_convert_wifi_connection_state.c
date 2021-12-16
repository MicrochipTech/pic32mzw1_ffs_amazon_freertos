/** @file ffs_convert_wifi_connection_state.c
 *
 * @brief Convert between API and DSS connection states implementation.
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
#include "ffs/conversion/ffs_convert_wifi_connection_state.h"

/*
 * Translate a DSS Wi-Fi connection state to a client-facing Wi-Fi connection state.
 */
FFS_RESULT ffsConvertDssWifiConnectionStateToApi(FFS_DSS_WIFI_CONNECTION_STATE dssState,
        FFS_WIFI_CONNECTION_STATE *apiState)
{
    switch(dssState) {
    case FFS_DSS_WIFI_CONNECTION_STATE_IDLE:
        *apiState = FFS_WIFI_CONNECTION_STATE_IDLE;
        break;
    case FFS_DSS_WIFI_CONNECTION_STATE_DISCONNECTED:
        *apiState = FFS_WIFI_CONNECTION_STATE_DISCONNECTED;
        break;
    case FFS_DSS_WIFI_CONNECTION_STATE_UNAUTHENTICATED:
        *apiState = FFS_WIFI_CONNECTION_STATE_UNAUTHENTICATED;
        break;
    case FFS_DSS_WIFI_CONNECTION_STATE_AUTHENTICATED:
        *apiState = FFS_WIFI_CONNECTION_STATE_AUTHENTICATED;
        break;
    case FFS_DSS_WIFI_CONNECTION_STATE_ASSOCIATED:
        *apiState = FFS_WIFI_CONNECTION_STATE_ASSOCIATED;
        break;
    case FFS_DSS_WIFI_CONNECTION_STATE_FAILED:
        *apiState = FFS_WIFI_CONNECTION_STATE_FAILED;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Translate a client-facing Wi-Fi connection state to a DSS Wi-Fi connection state.
 */
FFS_RESULT ffsConvertApiWifiConnectionStateToDss(FFS_WIFI_CONNECTION_STATE apiState,
        FFS_DSS_WIFI_CONNECTION_STATE *dssState)
{
    switch(apiState) {
    case FFS_WIFI_CONNECTION_STATE_IDLE:
        *dssState = FFS_DSS_WIFI_CONNECTION_STATE_IDLE;
        break;
    case FFS_WIFI_CONNECTION_STATE_DISCONNECTED:
        *dssState = FFS_DSS_WIFI_CONNECTION_STATE_DISCONNECTED;
        break;
    case FFS_WIFI_CONNECTION_STATE_UNAUTHENTICATED:
        *dssState = FFS_DSS_WIFI_CONNECTION_STATE_UNAUTHENTICATED;
        break;
    case FFS_WIFI_CONNECTION_STATE_AUTHENTICATED:
        *dssState = FFS_DSS_WIFI_CONNECTION_STATE_AUTHENTICATED;
        break;
    case FFS_WIFI_CONNECTION_STATE_ASSOCIATED:
        *dssState = FFS_DSS_WIFI_CONNECTION_STATE_ASSOCIATED;
        break;
    case FFS_WIFI_CONNECTION_STATE_FAILED:
        *dssState = FFS_DSS_WIFI_CONNECTION_STATE_FAILED;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
