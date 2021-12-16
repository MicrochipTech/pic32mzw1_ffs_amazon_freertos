/** @file ffs_dss_operation_post_wifi_scan_data.h
 *
 * @brief "Post Wi-Fi scan data" operation.
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

#ifndef FFS_DSS_OPERATION_POST_WIFI_SCAN_DATA_H_
#define FFS_DSS_OPERATION_POST_WIFI_SCAN_DATA_H_

#include "ffs/dss/model/ffs_dss_wifi_scan_result.h"
#include "ffs/dss/ffs_dss_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Callback to get Wi-Fi scan results.
 *
 * To add a scan result, call the function
 * \ref ffsDssPostWifiScanDataAddScanResult with the provided
 * \ref callbackDataPointer and the scan result to add.
 *
 * @param userContext User context
 * @param wifiScanResult Destination scan result object for the callback
 * @param callbackDataPointer Pointer to "get Wi-Fi scan results" data
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
typedef FFS_RESULT (*FfsDssGetWifiScanResultsCallback_t)(struct FfsUserContext_s *userContext,
        FfsDssWifiScanResult_t *wifiScanResult, void *callbackDataPointer);

/** @brief Execute a "post Wi-Fi scan data" operation.
 *
 * @param dssClientContext DSS client context
 * @param canProceed Can proceed flag
 * @param sequenceNumber Sequence number of this call
 * @param getScanResultsCallback Callback to retrieve the Wi-Fi scan results
 * @param wifiScanResult Destination scan result object for the callback
 * @param totalCredentialsFound Running total of credentials found across all calls
 * @param allCredentialsFound Boolean indicating all credentials have been found in the cloud
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssPostWifiScanData(FfsDssClientContext_t *dssClientContext,
        bool *canProceed,
        uint32_t sequenceNumber,
        FfsDssGetWifiScanResultsCallback_t getScanResultsCallback,
        FfsDssWifiScanResult_t *wifiScanResult,
        uint32_t *totalCredentialsFound,
        bool *allCredentialsFound);

/** @brief Add a Wi-Fi scan result to a "post Wi-Fi scan data" call.
 *
 * If this function returns \ref FFS_OVERRUN, the request has reached
 * the limit of scan results it can contain. In this case, exit the callback
 * and send this latest scan result in a subsequent request.
 *
 * @param callbackDataPointer Pointer to "post Wi-Fi scan data results" data
 * @param scanResult Scan result to add
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssPostWifiScanDataAddScanResult(void *callbackDataPointer,
        FfsDssWifiScanResult_t *scanResult);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_OPERATION_POST_WIFI_SCAN_DATA_H_ */
