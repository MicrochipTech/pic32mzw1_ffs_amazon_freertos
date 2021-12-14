/** @file ffs_wifi_scan_list.h
 *
 * @brief Ffs Wi-Fi scan list
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

#ifndef FFS_WIFI_SCAN_LIST_H_
#define FFS_WIFI_SCAN_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ffs/common/ffs_wifi.h"
#include "ffs/common/ffs_result.h"
#include "ffs/linux/ffs_wifi_context.h"

#include <stddef.h>

/** @brief Store the Wi-Fi scan result in the list by copy.
 *
 * @param wifiContext Wi-Fi context
 * @param scanResult The Wi-Fi configuration
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiScanListPush(FfsLinuxWifiContext_t *wifiContext, FfsWifiScanResult_t *scanResult);

/** @brief Retrieves the next Wi-Fi scan result in the list, without removing it.
 *
 * @param wifiContext Wi-Fi context
 * @param scanResult The next Wi-Fi configuration
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiScanListPeek(FfsLinuxWifiContext_t *wifiContext, FfsWifiScanResult_t **scanResult);

/** @brief Retrieves the Wi-Fi scan result at the given index, without removing it.
 *
 * @param wifiContext Wi-Fi context
 * @param index Index
 * @param scanResult The next Wi-Fi configuration
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiScanListPeekIndex(FfsLinuxWifiContext_t *wifiContext, size_t index, FfsWifiScanResult_t **scanResult);

/** @brief Removes the next Wi-Fi scan result from the list and frees the resources.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiScanListPop(FfsLinuxWifiContext_t *wifiContext);

/** @brief Update the scan list timestamp.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiScanListTouch(FfsLinuxWifiContext_t *wifiContext);

/** @brief Check if the Wi-Fi scan list is valid.
 *
 * @param wifiContext Wi-Fi context
 * @param isValid Destination boolean pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiScanListIsValid(FfsLinuxWifiContext_t *wifiContext, bool *isValid);

/** @brief Check if a network is in the Wi-Fi scan list.
 *
 * @param wifiContext Wi-Fi context
 * @param configuration The Wi-Fi configuration
 * @param hasNetwork Destination boolean pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiScanListHasNetwork(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *configuration,
        bool *hasNetwork);

/** @brief Removes all the Wi-Fi scan results from the list and frees the resources.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiScanListClear(FfsLinuxWifiContext_t *wifiContext);

/** @brief Get the size of the Wi-Fi scan list.
 *
 * @param wifiContext Wi-Fi context
 * @param size Destination size pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiScanListGetSize(FfsLinuxWifiContext_t *wifiContext, size_t *size);

/** @brief Checks if the Wi-Fi scan list is empty.
 *
 * @param wifiContext Wi-Fi context
 * @param isEmpty The result - true if empty; false otherwise
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiScanListIsEmpty(FfsLinuxWifiContext_t *wifiContext, bool *isEmpty);

#ifdef __cplusplus
}
#endif

#endif /* FFS_WIFI_SCAN_LIST_H_ */

