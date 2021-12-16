/** @file ffs_dss_wifi_security_protocol.h
 *
 * @brief DSS Wi-Fi security protocol enumeration.
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

#ifndef FFS_DSS_WIFI_SECURITY_PROTOCOL_H_
#define FFS_DSS_WIFI_SECURITY_PROTOCOL_H_

#include "ffs/common/ffs_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Wi-Fi security protocol enumeration.
 */
typedef enum {
    FFS_DSS_WIFI_SECURITY_PROTOCOL_OPEN = 0, //!< Open network.
    FFS_DSS_WIFI_SECURITY_PROTOCOL_WPA_PSK = 1, //!< WPA.
    FFS_DSS_WIFI_SECURITY_PROTOCOL_WEP = 2, //!< WEP.
    FFS_DSS_WIFI_SECURITY_PROTOCOL_OTHER = 3, //!< Other protocol.
} FFS_DSS_WIFI_SECURITY_PROTOCOL;

/** @brief Translate a DSS Wi-Fi security protocol to the DSS API model string.
 *
 * @param securityProtocol Enumerated security protocol
 * @param securityProtocolString Destination double pointer
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssGetWifiSecurityProtocolString(FFS_DSS_WIFI_SECURITY_PROTOCOL securityProtocol,
        const char **securityProtocolString);

/** @brief Parse a DSS model API string to a Ffs Wi-Fi security protocol.
 *
 * @param securityProtocolString Source string
 * @param securityProtocol Destination security protocol
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsDssParseWifiSecurityProtocol(const char *securityProtocolString,
        FFS_DSS_WIFI_SECURITY_PROTOCOL *securityProtocol);

#ifdef __cplusplus
}
#endif

#endif /* FFS_DSS_WIFI_SECURITY_PROTOCOL_H_ */
