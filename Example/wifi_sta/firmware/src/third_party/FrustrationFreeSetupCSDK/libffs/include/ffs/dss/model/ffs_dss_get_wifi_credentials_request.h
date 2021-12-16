/** @file ffs_dss_get_wifi_credentials_request.h
 *
 * @brief DSS "get Wi-Fi credentials" request.
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

#ifndef FFS_DSS_GET_WIFI_CREDENTIALS_REQUEST_H_
#define FFS_DSS_GET_WIFI_CREDENTIALS_REQUEST_H_

#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"
#include "ffs/dss/model/ffs_dss_device_details.h"
#include "ffs/dss/model/ffs_dss_wifi_credentials.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS "get Wi-Fi credentials" request.
 */
typedef struct {
    const char *nonce; //!< Nonce from the request.
    const char *sessionId; //!< Session ID.
    FfsDssDeviceDetails_t deviceDetails; //!< Device details.
    uint32_t sequenceNumber; //!< Sequence number.
} FfsDssGetWifiCredentialsRequest_t;

/** @brief Serialize a DSS "get Wi-Fi credentials" request.
 *
 * @param getWifiCredentialsRequest "get Wi-Fi credentials" request object to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeGetWifiCredentialsRequest(
        FfsDssGetWifiCredentialsRequest_t *getWifiCredentialsRequest, FfsStream_t *outputStream);

/** @brief Deserialize a DSS "get Wi-Fi credentials" request.
 *
 * @param getWifiCredentialsRequestValue Input JSON value
 * @param getWifiCredentialsRequest Destination "get Wi-Fi credentials" request object
 * @param wifiCredentialsListValue Destination for the JSON credentials list value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeGetWifiCredentialsRequest(
        FfsJsonValue_t *getWifiCredentialsRequestValue,
        FfsDssGetWifiCredentialsRequest_t *getWifiCredentialsRequest,
        FfsJsonValue_t *wifiCredentialsListValue);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_GET_WIFI_CREDENTIALS_REQUEST_H_ */
