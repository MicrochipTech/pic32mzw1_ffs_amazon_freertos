/** @file ffs_raspbian_wireless_tools.h
 *
 * @brief Raspbian wireless tools API.
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

#ifndef FFS_RASPBIAN_WIRELESS_TOOLS_H_
#define FFS_RASPBIAN_WIRELESS_TOOLS_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_wifi.h"
#include "ffs/linux/ffs_wifi_context.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Bring up a Wi-Fi interface.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianWifiInterfaceUp(FfsLinuxWifiContext_t *wifiContext);

/** @brief Tear down a Wi-Fi interface.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianWifiInterfaceDown(FfsLinuxWifiContext_t *wifiContext);

/** @brief Close a Wi-Fi connection on the given interface.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianCloseWifiConnection(FfsLinuxWifiContext_t *wifiContext);

/** @brief Connect to a Wi-Fi network using wireless tools.
 *
 * @param wifiContext Wi-Fi context
 * @param configuration Wi-Fi configuration
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianConnectToWifi(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration);

/** @brief Get the current Wi-Fi connection state, without modifying the internal state of the Wi-Fi context.
 *
 * This function does not modify the state of the Wi-Fi context, as caller functions are responsible
 * for determining failure reasons if an expected state is not returned.
 *
 * @param wifiContext Wi-Fi context
 * @param wifiConnectionState Destination state pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianGetWifiConnectionState(FfsLinuxWifiContext_t *wifiContext,
        FFS_WIFI_CONNECTION_STATE *wifiConnectionState);

#ifdef __cplusplus
}
#endif

#endif /* FFS_RASPBIAN_WIRELESS_TOOLS_H_ */
