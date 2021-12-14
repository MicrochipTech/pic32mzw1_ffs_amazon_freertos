/** @file ffs_dss_wifi_provisionee_state.h
 *
 * @brief DSS Wi-Fi provisionee state.
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

#ifndef FFS_DSS_WIFI_PROVISIONEE_STATE_H_
#define FFS_DSS_WIFI_PROVISIONEE_STATE_H_

#include "ffs/common/ffs_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS Wi-Fi provisionee states.
 */
typedef enum {
    FFS_DSS_WIFI_PROVISIONEE_STATE_NOT_PROVISIONED,
    FFS_DSS_WIFI_PROVISIONEE_STATE_START_PROVISIONING,
    FFS_DSS_WIFI_PROVISIONEE_STATE_START_PIN_BASED_SETUP,
    FFS_DSS_WIFI_PROVISIONEE_STATE_COMPUTE_CONFIGURATION,
    FFS_DSS_WIFI_PROVISIONEE_STATE_POST_WIFI_SCAN_DATA,
    FFS_DSS_WIFI_PROVISIONEE_STATE_GET_WIFI_LIST,
    FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTING_TO_USER_NETWORK,
    FFS_DSS_WIFI_PROVISIONEE_STATE_CONNECTED_TO_USER_NETWORK,
    FFS_DSS_WIFI_PROVISIONEE_STATE_DONE
} FFS_DSS_WIFI_PROVISIONEE_STATE;

/** @brief Translate a DSS Wi-Fi provisionee state to the DSS API model string.
 *
 * @param state Enumerated state
 * @param stateString Destination double pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssGetWifiProvisioneeStateString(FFS_DSS_WIFI_PROVISIONEE_STATE state,
        const char **stateString);

/** @brief Parse a DSS model API string to a Ffs Wi-Fi provisionee state.
 *
 * @param stateString Source string
 * @param state Destination provisionee state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssParseWifiProvisioneeState(const char *stateString,
        FFS_DSS_WIFI_PROVISIONEE_STATE *state);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_WIFI_PROVISIONEE_STATE_H_ */
