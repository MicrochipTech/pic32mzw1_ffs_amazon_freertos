/** @file ffs_dss_wifi_scan_result.h
 *
 * @brief DSS Wi-Fi scan result.
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

#ifndef FFS_DSS_WIFI_SCAN_RESULT_H_
#define FFS_DSS_WIFI_SCAN_RESULT_H_

#include "ffs/common/ffs_json.h"
#include "ffs/dss/model/ffs_dss_wifi_security_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Visible Wi-Fi network data structure.
 */
typedef struct {
    FfsStream_t ssidStream; //!< SSID.
    FfsStream_t bssidStream; //!< BSSID.
    FFS_DSS_WIFI_SECURITY_PROTOCOL securityProtocol; //!< Network security type.
    int32_t frequencyBand; //!< Frequency band.
    int32_t signalStrength; //!< Relative received signal strength in dB.
} FfsDssWifiScanResult_t;

/** @brief Serialize a DSS Wi-Fi scan result.
 *
 * @param wifiScanResult Wi-Fi scan result to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeWifiScanResult(FfsDssWifiScanResult_t *wifiScanResult,
        FfsStream_t *outputStream);

/** @brief Deserialize a DSS Wi-Fi scan result.
 *
 * @param getWifiCredentialsRequestValue Input JSON value
 * @param getWifiCredentialsRequest Destination "get Wi-Fi credentials" request object
 * @param wifiCredentialsListValue Destination for the JSON credentials list value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeWifiScanResult(FfsJsonValue_t *wifiScanResultValue,
        FfsDssWifiScanResult_t *wifiScanResult);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_WIFI_SCAN_RESULT_H_ */
