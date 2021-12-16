/** @file ffs_convert_wifi_provisionee_state.c
 *
 * @brief Convert between API and DSS provisionee states implementation.
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
#include "ffs/conversion/ffs_convert_wifi_provisionee_state.h"

/*
 * Translate a DSS Wi-Fi provisionee state to a client-facing Wi-Fi provisionee state.
 */
FFS_RESULT ffsConvertDssWifiProvisioneeStateToApi(FFS_DSS_WIFI_PROVISIONEE_STATE dssState,
        FFS_WIFI_PROVISIONEE_STATE *apiState)
{
    switch(dssState) {
    case FFS_DSS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED:
        *apiState = FFS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_START_PROVISIONING:
        *apiState = FFS_WIFI_PROVISIONEE_STATE_START_PROVISIONING;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP:
        *apiState = FFS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION:
        *apiState = FFS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA:
        *apiState = FFS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST:
        *apiState = FFS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK:
        *apiState = FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK:
        *apiState = FFS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_DONE:
        *apiState = FFS_WIFI_PROVISIONEE_STATE_DONE;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Translate a client-facing Wi-Fi provisionee state to a DSS Wi-Fi provisionee state.
 */
FFS_RESULT ffsConvertApiWifiProvisioneeStateToDss(FFS_WIFI_PROVISIONEE_STATE apiState,
        FFS_DSS_WIFI_PROVISIONEE_STATE *dssState)
{
    switch(apiState) {
    case FFS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED:
        *dssState = FFS_DSS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED;
        break;
    case FFS_WIFI_PROVISIONEE_STATE_START_PROVISIONING:
        *dssState = FFS_DSS_WIFI_PROVISIONEE_STATE_START_PROVISIONING;
        break;
    case FFS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP:
        *dssState = FFS_DSS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP;
        break;
    case FFS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION:
        *dssState = FFS_DSS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION;
        break;
    case FFS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA:
        *dssState = FFS_DSS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA;
        break;
    case FFS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST:
        *dssState = FFS_DSS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST;
        break;
    case FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK:
        *dssState = FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK;
        break;
    case FFS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK:
        *dssState = FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK;
        break;
    case FFS_WIFI_PROVISIONEE_STATE_DONE:
        *dssState = FFS_DSS_WIFI_PROVISIONEE_STATE_DONE;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
