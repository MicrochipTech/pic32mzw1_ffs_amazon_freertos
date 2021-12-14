/** @file ffs_wifi_context.h
 *
 * @brief Ffs Linux Wi-Fi context API
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

#ifndef FFS_LINUX_WIFI_CONTEXT_H_
#define FFS_LINUX_WIFI_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_wifi.h"
#include "ffs/compat/ffs_user_context.h"
#include "ffs/linux/ffs_linked_list.h"

#include <stdbool.h>
#include <pthread.h>
#include <time.h>

/** @brief Ffs Wi-Fi context structure
 */
typedef struct FfsWifiContext_s {
    bool wifiManagerInitialized; //!< Is the context initialized?
    FfsLinkedList_t configurationList; //!< Wi-Fi configuration list.
    FfsLinkedList_t scanList; //!< Wi-Fi scan list.
    pthread_mutex_t scanListMutex; //!< Scan list mutex.
    clock_t lastBackgroundScanTime; //!< Last background scan time.
    size_t scanListIndex; //!< Index for returning scan list items, reset by background scans.
    FfsLinkedList_t connectionAttemptList; //!< Wi-Fi connection attempts.
    const char *interface; //!< Wi-Fi interface to use (\a e.g. "wlan0").
    const char *driver; //!< Wi-Fi driver to use (\a e.g. "wext").
    uint8_t ssidBuffer[FFS_MAXIMUM_SSID_SIZE]; //!< Buffer for the connection details SSID.
    FfsWifiConnectionDetails_t connectionDetails; //!< Current Wi-Fi connection state.
} FfsLinuxWifiContext_t;

/** @brief Initialize a Ffs Wi-Fi context.
 *
 * @param wifiContext Ffs Wi-Fi context structure
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsInitializeWifiContext(FfsLinuxWifiContext_t *wifiContext);

/** @brief Deinitialize a Ffs Wi-Fi context.
 *
 * @param wifiContext Ffs Wi-Fi context structure
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDeinitializeWifiContext(FfsLinuxWifiContext_t *wifiContext);

/** @brief Update the Wi-Fi connection details.
 *
 * @param wifiContext Ffs Wi-Fi context structure
 * @param configuration Wi-Fi configuration
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsUpdateWifiConnectionDetails(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration);

/** @brief Update the Wi-Fi connection state.
 *
 * @param wifiContext Ffs Wi-Fi context structure
 * @param state Wi-Fi connection state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsUpdateWifiConnectionState(FfsLinuxWifiContext_t *wifiContext,
        FFS_WIFI_CONNECTION_STATE state);

/** @brief Update the Wi-Fi connection state in the failure case.
 *
 * @param wifiContext Ffs Wi-Fi context structure
 * @param errorDetails Error details object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsUpdateWifiConnectionFailure(FfsLinuxWifiContext_t *wifiContext,
        const FfsErrorDetails_t *errorDetails);

#ifdef __cplusplus
}
#endif

#endif /* FFS_LINUX_COMPAT_WIFI_CONTEXT_H_ */
