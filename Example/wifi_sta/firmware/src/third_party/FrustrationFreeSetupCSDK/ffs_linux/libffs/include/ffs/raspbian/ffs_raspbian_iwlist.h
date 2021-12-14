/** @file ffs_raspbian_iwlist.h
 *
 * @brief Raspbian Wi-Fi scanning using iwlist.
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

#ifndef FFS_RASPBIAN_IWLIST_H_
#define FFS_RASPBIAN_IWLIST_H_

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
FFS_RESULT ffsRaspbianPerformBackgroundScan(FfsLinuxWifiContext_t *wifiContext);

/** @brief Perform a directed Wi-Fi scan.
 *
 * @param wifiContext Wi-Fi context
 * @param configuration Network to scan for
 * @param isFound Destination boolean for scan result
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianPerformDirectedScan(FfsLinuxWifiContext_t *wifiContext,
        FfsWifiConfiguration_t *configuration, bool *isFound);

/** @brief Process the output of a background Wi-Fi scan command.
 *
 * @param output Shell command output
 * @param arg Argument pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianProcessBackgroundScanOutput(FILE *output, void *arg);

/** @brief Process the output of a directed Wi-Fi scan command.
 *
 * @param output Shell command output
 * @param arg Argument pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianProcessDirectedScanOutput(FILE *output, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* FFS_RASPBIAN_IWLIST_H_ */
