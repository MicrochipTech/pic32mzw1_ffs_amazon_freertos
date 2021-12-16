/** @file ffs_dss_post_wifi_scan_data_response.h
 *
 * @brief DSS "post Wi-Fi scan data" response.
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

#ifndef FFS_DSS_POST_WIFI_SCAN_DATA_RESPONSE_H_
#define FFS_DSS_POST_WIFI_SCAN_DATA_RESPONSE_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"
#include "ffs/compat/ffs_user_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "post Wi-Fi scan data" response.
 */
typedef struct {
    const char *nonce; //!< Nonce from the response.
    const char *sessionId; //!< Session ID.
    bool canProceed; //!< Can proceed?
    uint32_t sequenceNumber; //!< Sequence number.
    uint32_t totalCredentialsFound; //!< Number of total credentials found over all requests.
    bool allCredentialsFound; //!< Did the cloud find all the client's credentials?
} FfsDssPostWifiScanDataResponse_t;

/** @brief Serialize a DSS "post Wi-Fi scan data" response.
 *
 * @param postWifiScanDataResponse "post Wi-Fi scan data" response object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializePostWifiScanDataResponse(
        FfsDssPostWifiScanDataResponse_t *postWifiScanDataResponse,
        FfsStream_t *outputStream);

/** @brief Deserialize a DSS "post Wi-Fi scan data" response.
 *
 * @param postWifiScanDataResponseValue Input JSON value
 * @param postWifiScanDataResponse Destination "post Wi-Fi scan data" response object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializePostWifiScanDataResponse(
        FfsJsonValue_t *postWifiScanDataResponseValue,
        FfsDssPostWifiScanDataResponse_t *postWifiScanDataResponse);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_POST_WIFI_SCAN_DATA_RESPONSE_H_ */
