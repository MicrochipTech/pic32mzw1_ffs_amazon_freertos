/** @file ffs_macos_airport.h
 *
 * @brief macOS airport API.
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

#ifndef FFS_MACOS_AIRPORT_H_
#define FFS_MACOS_AIRPORT_H_

#include "ffs/common/ffs_wifi.h"
#include "ffs/linux/ffs_wifi_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Perform a background Wi-Fi scan.
 *
 * @param wifiContext Wi-Fi context
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsMacOsPerformBackgroundScan(FfsLinuxWifiContext_t *wifiContext);

/** @brief Perform a directed Wi-Fi scan.
 *
 * @param wifiContext Wi-Fi context
 * @param configuration Network to scan for
 * @param isFound Destination boolean pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsMacOsPerformDirectedScan(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration, bool *isFound);

#ifdef __cplusplus
}
#endif

#endif /* FFS_MACOS_AIRPORT_H_ */
