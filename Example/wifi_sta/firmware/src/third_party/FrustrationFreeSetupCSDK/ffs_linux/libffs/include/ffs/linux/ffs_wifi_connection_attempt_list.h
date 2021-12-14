/** @file ffs_wifi_connection_attempt_list.h
 *
 * @brief Ffs Wi-Fi connection attempt list
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

#ifndef FFS_WIFI_CONNECTION_ATTEMPT_LIST_H_
#define FFS_WIFI_CONNECTION_ATTEMPT_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ffs/linux/ffs_wifi_context.h"

#include <stddef.h>

/** @brief Store the Wi-Fi connection attempt in the list by copy.
 *
 * @param wifiContext Wi-Fi context
 * @param connectionAttempt The Wi-Fi connection attempt
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConnectionAttemptListPush(FfsLinuxWifiContext_t *wifiContext, FfsWifiConnectionAttempt_t *connectionAttempt);

/** @brief Retrieves the next Wi-Fi connection attempt in the list, without removing it.
 *
 * @param wifiContext Wi-Fi context
 * @param connectionAttempt The next Wi-Fi connection attempt
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConnectionAttemptListPeek(FfsLinuxWifiContext_t *wifiContext, FfsWifiConnectionAttempt_t **connectionAttempt);

/** @brief Removes the next Wi-Fi connection attempt from the list and frees the resources.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConnectionAttemptListPop(FfsLinuxWifiContext_t *wifiContext);

/** @brief Removes all the Wi-Fi connection attempts from the list and frees the resources.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConnectionAttemptListClear(FfsLinuxWifiContext_t *wifiContext);

/** @brief Checks if the Wi-Fi connection attempt list is empty.
 *
 * @param wifiContext Wi-Fi context
 * @param isEmpty The result - true if empty; false otherwise
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsWifiConnectionAttemptListIsEmpty(FfsLinuxWifiContext_t *wifiContext, bool *isEmpty);

#ifdef __cplusplus
}
#endif

#endif /* FFS_WIFI_CONNECTION_ATTEMPT_LIST_H_ */

