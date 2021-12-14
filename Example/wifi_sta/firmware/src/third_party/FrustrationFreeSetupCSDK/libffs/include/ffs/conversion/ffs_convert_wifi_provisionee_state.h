/** @file ffs_convert_wifi_provisionee_state.h
 *
 * @brief Convert between API and DSS provisionee states.
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

#ifndef FFS_CONVERT_WIFI_PROVISIONEE_STATE_H_
#define FFS_CONVERT_WIFI_PROVISIONEE_STATE_H_

#include "ffs/dss/model/ffs_dss_wifi_provisionee_state.h"
#include "ffs/wifi_provisionee/ffs_wifi_provisionee_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Translate a DSS Wi-Fi provisionee state to a client-facing Wi-Fi provisionee state.
 *
 * @param dssState Source DSS Wi-Fi provisionee state
 * @param apiState Destination API Wi-Fi provisionee state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertDssWifiProvisioneeStateToApi(FFS_DSS_WIFI_PROVISIONEE_STATE dssState,
        FFS_WIFI_PROVISIONEE_STATE *apiState);

/** @brief Translate a client-facing Wi-Fi provisionee state to a DSS Wi-Fi provisionee state.
 *
 * @param apiState Source API Wi-Fi provisionee state
 * @param dssState Destination DSS Wi-Fi provisionee state
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertApiWifiProvisioneeStateToDss(FFS_WIFI_PROVISIONEE_STATE apiState,
        FFS_DSS_WIFI_PROVISIONEE_STATE *dssState);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONVERT_WIFI_PROVISIONEE_STATE_H_ */
