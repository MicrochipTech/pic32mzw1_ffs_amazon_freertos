/** @file ffs_convert_wifi_connection_attempt.h
 *
 * @brief Convert between API and DSS Wi-Fi connection attempts.
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

#ifndef FFS_CONVERT_WIFI_CONNECTION_ATTEMPT_H_
#define FFS_CONVERT_WIFI_CONNECTION_ATTEMPT_H_

#include "ffs/common/ffs_wifi.h"
#include "ffs/dss/model/ffs_dss_wifi_connection_attempt.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Convert an API Wi-Fi connection attempt object to a DSS Wi-Fi connection attempt object.
 *
 * @param apiConnectionAttempt Source API connection attempt object
 * @param dssConnectionAttempt Destination connection attempt object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertApiWifiConnectionAttemptToDss(
        FfsWifiConnectionAttempt_t *apiConnectionAttempt,
        FfsDssWifiConnectionAttempt_t *dssConnectionAttempt);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONVERT_WIFI_CONNECTION_ATTEMPT_H_ */

