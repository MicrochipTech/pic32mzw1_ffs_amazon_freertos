/** @file ffs_dss_wifi_provisionee_state.h
 *
 * @brief DSS Wi-Fi provisionee state implementation.
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
#include "ffs/dss/model/ffs_dss_wifi_provisionee_state.h"

#define NOT_PROVISIONED_STRING                  "NOT_PROVISIONED"
#define START_PROVISIONING_STRING               "START_PROVISIONING"
#define START_PIN_BASED_SETUP_STRING            "START_PIN_BASED_SETUP"
#define POST_WIFI_SCAN_DATA_STRING              "POST_WIFI_SCAN_DATA"
#define GET_WIFI_LIST_STRING                    "GET_WIFI_LIST"
#define COMPUTE_CONFIGURATION_STRING            "COMPUTE_CONFIGURATION"
#define CONNECTING_TO_USER_NETWORK_STRING       "CONNECTING_TO_USER_NETWORK"
#define CONNECTED_TO_USER_NETWORK_STRING        "CONNECTED_TO_USER_NETWORK"
#define DONE_STRING                             "DONE"

/*
 * Translate a DSS Wi-Fi provisionee state to the DSS API model string.
 */
FFS_RESULT ffsDssGetWifiProvisioneeStateString(FFS_DSS_WIFI_PROVISIONEE_STATE state,
        const char **stateString)
{
    switch (state) {
    case FFS_DSS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED:
        *stateString = NOT_PROVISIONED_STRING;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_START_PROVISIONING:
        *stateString = START_PROVISIONING_STRING;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP:
        *stateString = START_PIN_BASED_SETUP_STRING;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION:
        *stateString = COMPUTE_CONFIGURATION_STRING;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA:
        *stateString = POST_WIFI_SCAN_DATA_STRING;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST:
        *stateString = GET_WIFI_LIST_STRING;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK:
        *stateString = CONNECTING_TO_USER_NETWORK_STRING;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK:
        *stateString = CONNECTED_TO_USER_NETWORK_STRING;
        break;
    case FFS_DSS_WIFI_PROVISIONEE_STATE_DONE:
        *stateString = DONE_STRING;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Parse a DSS model API string to a Ffs Wi-Fi provisionee state.
 */
FFS_RESULT ffsDssParseWifiProvisioneeState(const char *stateString,
        FFS_DSS_WIFI_PROVISIONEE_STATE *state)
{
    if (!strcmp(NOT_PROVISIONED_STRING, stateString)) {
        *state = FFS_DSS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED;
    } else if (!strcmp(START_PROVISIONING_STRING, stateString)) {
        *state = FFS_DSS_WIFI_PROVISIONEE_STATE_START_PROVISIONING;
    } else if (!strcmp(START_PIN_BASED_SETUP_STRING, stateString)) {
        *state = FFS_DSS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP;
    } else if (!strcmp(POST_WIFI_SCAN_DATA_STRING, stateString)) {
        *state = FFS_DSS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA;
    } else if (!strcmp(GET_WIFI_LIST_STRING, stateString)) {
        *state = FFS_DSS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST;
    } else if (!strcmp(COMPUTE_CONFIGURATION_STRING, stateString)) {
        *state = FFS_DSS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION;
    } else if (!strcmp(CONNECTING_TO_USER_NETWORK_STRING, stateString)) {
        *state = FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK;
    } else if (!strcmp(CONNECTED_TO_USER_NETWORK_STRING, stateString)) {
        *state = FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK;
    } else if (!strcmp(DONE_STRING, stateString)) {
        *state = FFS_DSS_WIFI_PROVISIONEE_STATE_DONE;
    } else {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
