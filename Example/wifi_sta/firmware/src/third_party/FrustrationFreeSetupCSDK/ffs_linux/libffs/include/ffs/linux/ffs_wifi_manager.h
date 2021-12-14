/** @file ffs_wifi_manager.h
 *
 * @brief Linux Wi-Fi manager API.
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * This file is a Modifiable File, as defined in the accompanying LICENSE.TXT
 * file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef FFS_WIFI_MANAGER_H_
#define FFS_WIFI_MANAGER_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_wifi.h"
#include "ffs/compat/ffs_user_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Wi-Fi manager callback type.
 */
typedef void(*FfsWifiManagerCallback_t)(struct FfsUserContext_s *userContext,
        FFS_RESULT result);

/** @brief Initialize the Wi-Fi manager.
 *
 * @param userContext User context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsInitializeWifiManager(struct FfsUserContext_s *userContext);

/** @brief Deinitialize the Wi-Fi manager.
 *
 * @param userContext User context
 * @param callback Callback to be executed on deinitialization
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDeinitializeWifiManager(struct FfsUserContext_s *userContext,
        FfsWifiManagerCallback_t callback);

/** @brief Enqueue an event to begin a Wi-Fi scan.
 *
 * @param callback Callback to be executed on scan completion
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiManagerStartScan(FfsWifiManagerCallback_t callback);

/** @brief Enqueue an event to connect to a WEP network.
 *
 * @param wifiConfiguration Wi-Fi configuration to connect to
 * @param hostNameStream Host name to resolve
 * @param callback Callback to be executed after connection attempt
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiManagerConnectToWepNetwork(FfsWifiConfiguration_t *wifiConfiguration,
        FfsStream_t *hostNameStream, FfsWifiManagerCallback_t callback);

/** @brief Enqueue an event to connect to a network.
 *
 * @param wifiConfiguration Wi-Fi configuration to connect to
 * @param wpaSupplicantConfigurationFile WPA supplicant configuration file to use
 * @param hostNameStream Host name to resolve
 * @param callback Callback to be executed after connection attempt
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiManagerConnectToNetwork(FfsWifiConfiguration_t *wifiConfiguration,
        const char *wpaSupplicantConfigurationFile, FfsStream_t *hostNameStream,
        FfsWifiManagerCallback_t callback);

/** @brief Enqueue an event to connect to networks from the configuration list.
 *
 * Attempt to connect to networks in the configuration list until a connection
 * succeeds, or until all networks have been tried.
 *
 * @param wpaSupplicantConfigurationFile WPA supplicant configuration file to use
 * @param hostNameStream Host name to resolve
 * @param callback Callback to be executed after connection attempts
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiManagerConnect(const char *wpaSupplicantConfigurationFile,
        FfsStream_t *hostNameStream, FfsWifiManagerCallback_t callback);

/** @brief Enqueue an event to disconnect from Wi-Fi.
 *
 * @param callback Callback to be executed after disconnection
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiManagerDisconnect(FfsWifiManagerCallback_t callback);

#ifdef __cplusplus
}
#endif

#endif /* FFS_WIFI_MANAGER_H_ */
