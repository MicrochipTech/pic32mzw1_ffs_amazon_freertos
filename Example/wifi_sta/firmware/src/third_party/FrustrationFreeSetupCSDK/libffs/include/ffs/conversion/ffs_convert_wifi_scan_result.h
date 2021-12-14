/** @file ffs_convert_wifi_credentials.h
 *
 * @brief Convert between API and DSS Wi-Fi scan results.
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

#ifndef FFS_CONVERT_WIFI_SCAN_RESULT_H_
#define FFS_CONVERT_WIFI_SCAN_RESULT_H_

#include "ffs/common/ffs_wifi.h"
#include "ffs/dss/model/ffs_dss_wifi_scan_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Convert an API Wi-Fi scan result to a DSS Wi-Fi scan result.
 *
 * @param apiWifiScanResult Source API scan result
 * @param dssWifiScanResult Destination DSS scan result
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertApiWifiScanResultToDss(FfsWifiScanResult_t *apiWifiScanResult,
        FfsDssWifiScanResult_t *dssWifiScanResult);

/** @brief Convert a DSS Wi-Fi scan result to an API Wi-Fi scan result.
 *
 * @param dssWifiScanResult Source DSS scan result
 * @param apiWifiScanResult Destination API scan result
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertDssWifiScanResultToApi(FfsDssWifiScanResult_t *dssWifiScanResult,
        FfsWifiScanResult_t *apiWifiScanResult);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONVERT_WIFI_SCAN_RESULT_H_ */
