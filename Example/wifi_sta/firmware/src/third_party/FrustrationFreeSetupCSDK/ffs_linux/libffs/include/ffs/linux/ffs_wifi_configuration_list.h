/** @file ffs_wifi_configuration_list.h
 *
 * @brief Ffs Wi-Fi configuration list
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

#ifndef FFS_WIFI_CONFIGURATION_LIST_H_
#define FFS_WIFI_CONFIGURATION_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ffs/linux/ffs_wifi_context.h"

#include <stddef.h>

/** @brief Store the Wi-Fi configuration in the list by copy.
 *
 * @param wifiContext Wi-Fi context
 * @param configuration The Wi-Fi configuration
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConfigurationListPush(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *configuration);

/** @brief Retrieves the next Wi-Fi configuration in the list, without removing it.
 *
 * @param wifiContext Wi-Fi context
 * @param configuration The next Wi-Fi configuration
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConfigurationListPeek(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t **configuration);

/** @brief Removes the next Wi-Fi configuration from the list and frees the resources.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConfigurationListPop(FfsLinuxWifiContext_t *wifiContext);

/** @brief Removes a Wi-Fi configuration from the list by SSID.
 *
 * @param wifiContext Wi-Fi context
 * @param ssidStream SSID to remove
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConfigurationListPopConfiguration(FfsLinuxWifiContext_t *wifiContext, FfsStream_t ssidStream);

/** @brief Removes all the Wi-Fi configurations from the list and frees the resources.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConfigurationListClear(FfsLinuxWifiContext_t *wifiContext);

/** @brief Checks if the Wi-Fi configuration list is empty.
 *
 * @param wifiContext Wi-Fi context
 * @param isEmpty The result - true if empty; false otherwise
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConfigurationListIsEmpty(FfsLinuxWifiContext_t *wifiContext, bool *isEmpty);

#ifdef __cplusplus
}
#endif

#endif /* FFS_WIFI_CONFIGURATION_LIST_H_ */
