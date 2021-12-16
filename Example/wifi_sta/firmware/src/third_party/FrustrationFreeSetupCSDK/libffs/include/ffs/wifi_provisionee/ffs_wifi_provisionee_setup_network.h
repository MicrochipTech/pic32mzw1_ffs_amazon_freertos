/** @file ffs_wifi_provisionee_setup_network.h
 *
 * @brief Wi-Fi FFS setup network.
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

#ifndef FFS_WIFI_PROVISIONEE_SETUP_NETWORK_H_
#define FFS_WIFI_PROVISIONEE_SETUP_NETWORK_H_

#include "ffs/common/ffs_result.h"
#include "ffs/compat/ffs_user_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Connect to the Wi-Fi FFS setup network.
 *
 * @param userContext User context
 * @param setupWifiConfiguration The wifi configuration to use.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConnectToSetupNetwork(struct FfsUserContext_s *userContext, FfsWifiConfiguration_t *setupWifiConfiguration);

/** @brief Disconnect from the Wi-Fi FFS setup network.
 *
 * @param userContext User context
 * @param setupWifiConfiguration The wifi configuration to use.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDisconnectFromSetupNetwork(struct FfsUserContext_s *userContext, FfsWifiConfiguration_t *setupWifiConfiguration);

/** @brief Get the default Wi-Fi FFS setup network configuration.
 *
 * @param wifiConfiguration Destination configuration object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetDefaultSetupNetworkConfiguration(FfsWifiConfiguration_t *wifiConfiguration);

/** @brief Get the Wi-Fi FFS setup network.
 *
 * @param userContext User context
 * @param wifiConfiguration The destination wifi configuration.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetFallbackSetupNetwork(struct FfsUserContext_s *userContext, FfsWifiConfiguration_t *wifiConfiguration);

#ifdef __cplusplus
}
#endif

#endif /* FFS_WIFI_PROVISIONEE_SETUP_NETWORK_H_ */
