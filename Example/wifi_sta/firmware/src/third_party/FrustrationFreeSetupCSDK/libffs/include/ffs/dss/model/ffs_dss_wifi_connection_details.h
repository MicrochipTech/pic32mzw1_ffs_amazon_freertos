/** @file ffs_dss_wifi_connection_details.h
 *
 * @brief DSS Wi-Fi connection details.
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

#ifndef FFS_DSS_WIFI_CONNECTION_DETAILS_H_
#define FFS_DSS_WIFI_CONNECTION_DETAILS_H_

#include "ffs/common/ffs_json.h"
#include "ffs/dss/model/ffs_dss_error_details.h"
#include "ffs/dss/model/ffs_dss_wifi_connection_state.h"
#include "ffs/dss/model/ffs_dss_wifi_security_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Wi-Fi connection details.
 */
typedef struct {
    FfsStream_t ssidStream; //!< SSID.
    FFS_DSS_WIFI_SECURITY_PROTOCOL securityProtocol; //!< Network security type.
    FFS_DSS_WIFI_CONNECTION_STATE state; //!< Wi-Fi connection state.
    bool hasErrorDetails; //!< Has an error details field.
    FfsDssErrorDetails_t errorDetails; //!< Optional error details.
} FfsDssWifiConnectionDetails_t;

/** @brief Serialize DSS Wi-Fi connection details.
 *
 * @param wifiConnectionDetails Wi-Fi connection details to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeWifiConnectionDetails(
        FfsDssWifiConnectionDetails_t *wifiConnectionDetails, FfsStream_t *outputStream);

/** @brief Deserialize DSS Wi-Fi connection details.
 *
 * @param getWifiConnectionDetailsValue Input JSON value
 * @param getWifiConnectionDetails Destination "Wi-Fi connection details" object
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeWifiConnectionDetails(FfsJsonValue_t *wifiConnectionDetailsValue,
        FfsDssWifiConnectionDetails_t *wifiConnectionDetails);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_WIFI_CONNECTION_DETAILS_H_ */
