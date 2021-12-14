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

#include "ffs/common/ffs_check_result.h"
#include "ffs/conversion/ffs_convert_wifi_security_protocol.h"

/*
 * Translate a DSS Wi-Fi security protocol to a client-facing Wi-Fi security protocol.
 */
FFS_RESULT ffsConvertDssWifiSecurityProtocolToApi(FFS_DSS_WIFI_SECURITY_PROTOCOL dssProtocol,
        FFS_WIFI_SECURITY_PROTOCOL *apiProtocol)
{
    switch(dssProtocol) {
    case FFS_DSS_WIFI_SECURITY_PROTOCOL_OPEN:
        *apiProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
        break;
    case FFS_DSS_WIFI_SECURITY_PROTOCOL_WPA_PSK:
        *apiProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
        break;
    case FFS_DSS_WIFI_SECURITY_PROTOCOL_WEP:
        *apiProtocol = FFS_WIFI_SECURITY_PROTOCOL_WEP;
        break;
    case FFS_DSS_WIFI_SECURITY_PROTOCOL_OTHER:
        *apiProtocol = FFS_WIFI_SECURITY_PROTOCOL_OTHER;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Translate a client-facing Wi-Fi security protocol to a DSS Wi-Fi security protocol.
 */
FFS_RESULT ffsConvertApiWifiSecurityProtocolToDss(FFS_WIFI_SECURITY_PROTOCOL apiProtocol,
        FFS_DSS_WIFI_SECURITY_PROTOCOL *dssProtocol)
{
    switch(apiProtocol) {
    case FFS_WIFI_SECURITY_PROTOCOL_NONE:
        *dssProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_OPEN;
        break;
    case FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK:
        *dssProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
        break;
    case FFS_WIFI_SECURITY_PROTOCOL_WEP:
        *dssProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_WEP;
        break;
    case FFS_WIFI_SECURITY_PROTOCOL_OTHER:
        *dssProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_OTHER;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
