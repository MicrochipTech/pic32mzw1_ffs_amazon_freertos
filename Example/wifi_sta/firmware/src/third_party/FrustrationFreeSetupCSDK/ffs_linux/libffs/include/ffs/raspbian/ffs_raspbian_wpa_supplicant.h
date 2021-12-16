/** @file ffs_raspbian_wpa_supplicant.h
 *
 * @brief Raspbian WPA supplicant API.
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

#ifndef FFS_RASPBIAN_WPA_SUPPLICANT_H_
#define FFS_RASPBIAN_WPA_SUPPLICANT_H_

#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_wifi.h"
#include "ffs/linux/ffs_wifi_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Kill the WPA supplicant to avoid conflicts with wireless tools.
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianKillWpaSupplicant();

/** @brief Write to a configuration file for the WPA supplicant
 *
 * @param configuration Wi-Fi configuration
 * @param configurationFile WPA supplicant configuration file to use
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianConfigureWpaSupplicant(FfsWifiConfiguration_t *configuration, const char *configurationFile);

/** @brief Connect to a Wi-Fi network using the WPA supplicant.
 *
 * @param wifiContext Wi-Fi context
 * @param configuration Wi-Fi configuration
 * @param configurationFile WPA supplicant configuration file to use
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsRaspbianConnectWithWpaSupplicant(FfsLinuxWifiContext_t *wifiContext, FfsWifiConfiguration_t *configuration,
        const char *configurationFile);

#ifdef __cplusplus
}
#endif

#endif /* FFS_RASPBIAN_WPA_SUPPLICANT_H_ */
