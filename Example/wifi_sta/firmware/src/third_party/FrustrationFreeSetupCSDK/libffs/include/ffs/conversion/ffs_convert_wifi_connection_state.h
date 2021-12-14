/** @file ffs_convert_wifi_connection_state.h
 *
 * @brief Convert between API and DSS connection states.
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

#ifndef FFS_CONVERT_WIFI_CONNECTION_STATE_H_
#define FFS_CONVERT_WIFI_CONNECTION_STATE_H_

#include "ffs/common/ffs_wifi.h"
#include "ffs/dss/model/ffs_dss_wifi_connection_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Translate a DSS Wi-Fi connection state to a client-facing Wi-Fi connection state.
 *
 * @param dssState Source DSS Wi-Fi connection state
 * @param apiState Destination API Wi-Fi connection state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertDssWifiConnectionStateToApi(FFS_DSS_WIFI_CONNECTION_STATE dssState,
        FFS_WIFI_CONNECTION_STATE *apiState);

/** @brief Translate a client-facing Wi-Fi connection state to a DSS Wi-Fi connection state.
 *
 * @param apiState Source API Wi-Fi connection state
 * @param dssState Destination DSS Wi-Fi connection state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertApiWifiConnectionStateToDss(FFS_WIFI_CONNECTION_STATE apiState,
        FFS_DSS_WIFI_CONNECTION_STATE *dssState);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONVERT_WIFI_CONNECTION_STATE_H_ */
