/** @file ffs_dss_wifi_credentials.h
 *
 * @brief DSS Wi-Fi credentials serialization/deserialization.
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

#ifndef FFS_DSS_WIFI_CREDENTIALS_H_
#define FFS_DSS_WIFI_CREDENTIALS_H_

#include "ffs/common/ffs_json.h"
#include "ffs/dss/model/ffs_dss_wifi_security_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DSS Wi-Fi credentials.
 */
typedef struct {
    FfsStream_t ssidStream; //!< SSID.
    FFS_DSS_WIFI_SECURITY_PROTOCOL securityProtocol; //!< Network security type.
    FfsStream_t keyStream; //!< WEP/WPA key.
    int32_t wepIndex; //!< WEP index. @deprecated
    int32_t networkPriority; //!< Network priority. @deprecated
    int32_t frequency; //!< Frequency in MHz.
} FfsDssWifiCredentials_t;

/** @brief Serialize DSS Wi-Fi credentials.
 *
 * @param wifiCredentials Wi-Fi credentials to serialize
 * @param outputStream Output stream
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssSerializeWifiCredentials(FfsDssWifiCredentials_t *wifiCredentials,
        FfsStream_t *outputStream);

/** @brief Deserialize DSS Wi-Fi credentials.
 *
 * @param getWifiCredentialsRequestValue Input JSON value
 * @param getWifiCredentialsRequest Destination "get Wi-Fi credentials" request object
 * @param wifiCredentialsListValue Destination for the JSON credentials list value
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssDeserializeWifiCredentials(FfsJsonValue_t *wifiCredentialsValue,
        FfsDssWifiCredentials_t *wifiCredentials);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_WIFI_CREDENTIALS_H_ */
