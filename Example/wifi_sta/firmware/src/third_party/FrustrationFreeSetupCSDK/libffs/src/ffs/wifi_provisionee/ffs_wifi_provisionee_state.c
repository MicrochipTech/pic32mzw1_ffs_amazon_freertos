/** @file ffs_wifi_provisionee_state.c
 *
 * @brief Ffs provisionee state implementation.
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
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_state.h"

/*
 *  Translate a Ffs Wi-Fi provisionee state to a readable string.
 */
FFS_RESULT ffsGetWifiProvisioneeStateString(FFS_WIFI_PROVISIONEE_STATE state,
        const char **stateString)
{
    switch (state) {
        case FFS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED:
            *stateString = "NOT_PROVISIONED";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_SETUP_NETWORK:
            *stateString = "CONNECTING_TO_SETUP_NETWORK";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_START_PROVISIONING:
            *stateString = "START_PROVISIONING";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP:
            *stateString = "START_PIN_BASED_SETUP";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION:
            *stateString = "COMPUTE_CONFIGURATION";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA:
            *stateString = "POST_WIFI_SCAN_DATA";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST:
            *stateString = "GET_WIFI_LIST";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK:
            *stateString = "CONNECTING_TO_USER_NETWORK";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK:
            *stateString = "CONNECTED_TO_USER_NETWORK";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_DONE:
            *stateString = "DONE";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_FAILURE:
            *stateString = "FAILURE";
            return FFS_SUCCESS;
        case FFS_WIFI_PROVISIONEE_STATE_TERMINATED:
            *stateString = "TERMINATED";
            return FFS_SUCCESS;
        default:
            FFS_FAIL(FFS_ERROR);
    }
}

/*
 *  Check if a Ffs Wi-Fi provisionee state is terminal.
 */
bool ffsWifiProvisioneeStateIsTerminal(FFS_WIFI_PROVISIONEE_STATE state)
{
    switch (state) {
        case FFS_WIFI_PROVISIONEE_STATE_DONE:
        case FFS_WIFI_PROVISIONEE_STATE_FAILURE:
        case FFS_WIFI_PROVISIONEE_STATE_TERMINATED:
            return true;
        default:
            return false;
    }
}
