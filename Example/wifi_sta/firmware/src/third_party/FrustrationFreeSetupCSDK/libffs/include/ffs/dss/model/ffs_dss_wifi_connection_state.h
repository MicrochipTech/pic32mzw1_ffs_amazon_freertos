/** @file ffs_dss_wifi_connection_state.h
 *
 * @brief DSS Wi-Fi connection state enumeration.
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

#ifndef FFS_DSS_WIFI_CONNECTION_STATE_H_
#define FFS_DSS_WIFI_CONNECTION_STATE_H_

#include "ffs/common/ffs_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Wi-Fi connection state enumeration.
 */
typedef enum {
    FFS_DSS_WIFI_CONNECTION_STATE_IDLE = 0, //!< Initial Wi-Fi connection state.
    FFS_DSS_WIFI_CONNECTION_STATE_DISCONNECTED = 2, //!< Disconnected from Wi-Fi.
    FFS_DSS_WIFI_CONNECTION_STATE_UNAUTHENTICATED = 3, //!< Initial state of a Wi-Fi connection attempt.
    FFS_DSS_WIFI_CONNECTION_STATE_AUTHENTICATED = 4, //!< Authenticated with a Wi-Fi network.
    FFS_DSS_WIFI_CONNECTION_STATE_ASSOCIATED = 5, //!< Associated with (connected to) a Wi-Fi network.
    FFS_DSS_WIFI_CONNECTION_STATE_FAILED = -1 //!< Wi-Fi connection failed.
} FFS_DSS_WIFI_CONNECTION_STATE;

/** @brief Translate a DSS Wi-Fi connection state to the DSS API model string.
 *
 * @param connectionState Enumerated connection state
 * @param connectionStateString Destination double pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssGetWifiConnectionStateString(FFS_DSS_WIFI_CONNECTION_STATE connectionState,
        const char **connectionStateString);

/** @brief Parse a DSS model API string to a Ffs Wi-Fi connection state.
 *
 * @param connectionStateString Source string
 * @param connectionState Destination connection state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssParseWifiConnectionState(const char *connectionStateString,
        FFS_DSS_WIFI_CONNECTION_STATE *connectionState);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_WIFI_CONNECTION_STATE_H_ */
