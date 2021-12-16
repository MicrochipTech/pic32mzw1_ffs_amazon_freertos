/** @file ffs_convert_wifi_security_protocol.h
 *
 * @brief Convert between API and DSS security protocols.
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

#ifndef FFS_CONVERT_WIFI_SECURITY_PROTOCOL_H_
#define FFS_CONVERT_WIFI_SECURITY_PROTOCOL_H_

#include "ffs/common/ffs_wifi.h"
#include "ffs/dss/model/ffs_dss_wifi_security_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Translate a DSS Wi-Fi security protocol to a client-facing Wi-Fi security protocol.
 *
 * @param dssProtocol Source DSS Wi-Fi security protocol
 * @param apiProtocol Destination API Wi-Fi security protocol
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertDssWifiSecurityProtocolToApi(FFS_DSS_WIFI_SECURITY_PROTOCOL dssProtocol,
        FFS_WIFI_SECURITY_PROTOCOL *apiProtocol);

/** @brief Translate a client-facing Wi-Fi security protocol to a DSS Wi-Fi security protocol.
 *
 * @param apiProtocol Source API Wi-Fi security protocol
 * @param dssProtocol Destination DSS Wi-Fi security protocol
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsConvertApiWifiSecurityProtocolToDss(FFS_WIFI_SECURITY_PROTOCOL apiProtocol,
        FFS_DSS_WIFI_SECURITY_PROTOCOL *dssProtocol);

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONVERT_WIFI_SECURITY_PROTOCOL_H_ */
