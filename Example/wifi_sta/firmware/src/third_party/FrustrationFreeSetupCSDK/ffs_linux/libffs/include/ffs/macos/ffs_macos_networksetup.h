/** @file ffs_macos_networksetup.h
 *
 * @brief macOS "networksetup" API.
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

#ifndef FFS_MACOS_NETWORKSETUP_H_
#define FFS_MACOS_NETWORKSETUP_H_

#include "ffs/linux/ffs_wifi_context.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Close a Wi-Fi connection on the given interface.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsMacOsCloseWifiConnection(FfsLinuxWifiContext_t *wifiContext);

/** @brief Connect to a Wi-Fi network using "networksetup".
 *
 * @param wifiContext Wi-Fi context
 * @param configuration Wi-Fi configuration
 * @param configurationFile Configuration file (not used)
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsMacOsConnectToWifi(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration);

/** @brief Get the current Wi-Fi connection state.
 *
 * @param wifiContext Wi-Fi context
 * @param wifiConfiguration Wi-Fi configuration
 * @param wifiConnectionState Destination Wi-Fi state pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsMacOsGetWifiConnectionState(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *wifiConfiguration,
        FFS_WIFI_CONNECTION_STATE *wifiConnectionState);

#ifdef __cplusplus
}
#endif

#endif /* FFS_MACOS_NETWORKSETUP_H_ */
