/** @file ffs_dss_post_wifi_scan_data_request.h
 *
 * @brief DSS "post Wi-Fi scan data" request.
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

#ifndef FFS_DSS_POST_WIFI_SCAN_DATA_REQUEST_H_
#define FFS_DSS_POST_WIFI_SCAN_DATA_REQUEST_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"
#include "ffs/dss/model/ffs_dss_device_details.h"
#include "ffs/dss/model/ffs_dss_wifi_scan_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "post Wi-Fi scan data" request (excluding scan results).
 *
 * The scan results are added to the http request body separately.
 */
typedef struct {
    const char *nonce; //!< Nonce from the request.
    const char *sessionId; //!< Session ID.
    FfsDssDeviceDetails_t deviceDetails; //!< Device details.
    uint32_t sequenceNumber; //!< Sequence number.
} FfsDssPostWifiScanDataRequest_t;

/** @brief Start serializing a DSS "post Wi-Fi scan data" request.
 *
 * @param getWifiCredentialsRequest "post Wi-Fi scan data" request object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssStartSerializingPostWifiScanDataRequest(
        FfsDssPostWifiScanDataRequest_t *getWifiCredentialsRequest, FfsStream_t *outputStream);

/** @brief Add a scan result to a DSS "post Wi-Fi scan data" request.
 *
 * Add a scan result to the request. The call will return
 * \ref FFS_OVERRUN if the serialized scan result does not fit within
 * the output stream or if there will be insufficient space remaining to
 * complete the request. In either case the output stream will be left
 * unchanged from prior to the call.
 *
 * @param scanResult Scan result object to add
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssAddScanResultToSerializedPostWifiScanDataRequest(
        FfsDssWifiScanResult_t *scanResult, FfsStream_t *outputStream);

/** @brief Complete serializing a DSS "post Wi-Fi scan data" request.
 *
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssFinishSerializingPostWifiScanDataRequest(FfsStream_t *outputStream);

/** @brief Deserialize a DSS "post Wi-Fi scan data" request.
 *
 * @param postWifiScanDataRequestValue Input JSON value
 * @param postWifiScanDataRequest Destination "post Wi-Fi scan data" request object
 * @param wifiScanDataValue Destination for the JSON Wi-Fi scan data list value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializePostWifiScanDataRequest(
        FfsJsonValue_t *postWifiScanDataRequestValue,
        FfsDssPostWifiScanDataRequest_t *postWifiScanDataRequest,
        FfsJsonValue_t *wifiScanDataValue);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_POST_WIFI_SCAN_DATA_REQUEST_H_ */

