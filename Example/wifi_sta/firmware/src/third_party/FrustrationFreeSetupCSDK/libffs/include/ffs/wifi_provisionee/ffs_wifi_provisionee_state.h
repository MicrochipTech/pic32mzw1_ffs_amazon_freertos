/** @file ffs_wifi_provisionee_state.h
 *
 * @brief Ffs Wi-Fi provisionee state enumeration.
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

#ifndef FFS_WIFI_PROVISIONEE_STATE_H_
#define FFS_WIFI_PROVISIONEE_STATE_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Ffs Wi-Fi provisionee states.
 */
typedef enum {
    FFS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED,
    FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_SETUP_NETWORK,
    FFS_WIFI_PROVISIONEE_STATE_START_PROVISIONING,
    FFS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP,
    FFS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION,
    FFS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA,
    FFS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST,
    FFS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK,
    FFS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK,
    FFS_WIFI_PROVISIONEE_STATE_DONE,
    FFS_WIFI_PROVISIONEE_STATE_FAILURE,
    FFS_WIFI_PROVISIONEE_STATE_TERMINATED
} FFS_WIFI_PROVISIONEE_STATE;

/** @brief Translate a Ffs Wi-Fi provisionee state to a readable string.
 *
 * @param state Enumerated state
 * @param stateString Destination double pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetWifiProvisioneeStateString(FFS_WIFI_PROVISIONEE_STATE state,
        const char **stateString);

/** @brief Check if a Ffs Wi-Fi provisionee state is terminal.
 *
 * @param state Enumerated state
 *
 * @returns true if the state is terminal
 */
bool ffsWifiProvisioneeStateIsTerminal(FFS_WIFI_PROVISIONEE_STATE state);

#ifdef __cplusplus
}
#endif

#endif /* FFS_WIFI_PROVISIONEE_STATE_H_ */
