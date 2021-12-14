/** @file ffs_dss_wifi_connection_attempt.h
 *
 * @brief DSS Wi-Fi connection attempt.
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef FFS_DSS_WIFI_CONNECTION_ATTEMPT_H_
#define FFS_DSS_WIFI_CONNECTION_ATTEMPT_H_

#include "ffs/dss/model/ffs_dss_wifi_connection_details.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Wi-Fi connection attempt.
 */
typedef FfsDssWifiConnectionDetails_t FfsDssWifiConnectionAttempt_t;

/** @brief Serialize DSS Wi-Fi connection attempt.
 *
 * @param wifiConnectionAttempt Wi-Fi connection attempt to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeWifiConnectionAttempt(
        FfsDssWifiConnectionAttempt_t *wifiConnectionAttempt, FfsStream_t *outputStream);

/** @brief Deserialize DSS Wi-Fi connection attempt.
 *
 * @param getWifiConnectionAttemptValue Input JSON value
 * @param getWifiConnectionAttempt Destination "Wi-Fi connection attempt" object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeWifiConnectionAttempt(FfsJsonValue_t *wifiConnectionAttemptValue,
        FfsDssWifiConnectionAttempt_t *wifiConnectionAttempt);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_WIFI_CONNECTION_ATTEMPT_H_ */

