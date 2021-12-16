/** @file ffs_dss_wifi_security_protocol.c
 *
 * @brief DSS Wi-Fi security protocol enumeration implementation.
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
#include "ffs/dss/model/ffs_dss_wifi_security_protocol.h"

#include <string.h>

#define SECURITY_PROTOCOL_OPEN_STRING           "OPEN"
#define SECURITY_PROTOCOL_WPA_PSK_STRING        "WPA_PSK"
#define SECURITY_PROTOCOL_WEP_STRING            "WEP"
#define SECURITY_PROTOCOL_OTHER_STRING          "OTHER"

/*
 * Translate a DSS Wi-Fi security protocol to the DSS API model string.
 */
FFS_RESULT ffsDssGetWifiSecurityProtocolString(FFS_DSS_WIFI_SECURITY_PROTOCOL securityProtocol,
        const char **securityProtocolString)
{
    switch (securityProtocol) {
    case FFS_DSS_WIFI_SECURITY_PROTOCOL_OPEN:
        *securityProtocolString = SECURITY_PROTOCOL_OPEN_STRING;
        break;
    case FFS_DSS_WIFI_SECURITY_PROTOCOL_WPA_PSK:
        *securityProtocolString = SECURITY_PROTOCOL_WPA_PSK_STRING;
        break;
    case FFS_DSS_WIFI_SECURITY_PROTOCOL_WEP:
        *securityProtocolString = SECURITY_PROTOCOL_WEP_STRING;
        break;
    case FFS_DSS_WIFI_SECURITY_PROTOCOL_OTHER:
        *securityProtocolString = SECURITY_PROTOCOL_OTHER_STRING;
        break;
    default:
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}

/*
 * Parse a DSS model API string to a Ffs Wi-Fi security protocol.
 */
FFS_RESULT ffsDssParseWifiSecurityProtocol(const char *securityProtocolString,
        FFS_DSS_WIFI_SECURITY_PROTOCOL *securityProtocol)
{
    if (!strcmp(SECURITY_PROTOCOL_OPEN_STRING, securityProtocolString)) {
        *securityProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_OPEN;
    } else if (!strcmp(SECURITY_PROTOCOL_WPA_PSK_STRING, securityProtocolString)) {
        *securityProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_WPA_PSK;
    } else if (!strcmp(SECURITY_PROTOCOL_WEP_STRING, securityProtocolString)) {
        *securityProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_WEP;
    } else if (!strcmp(SECURITY_PROTOCOL_OTHER_STRING, securityProtocolString)) {
        *securityProtocol = FFS_DSS_WIFI_SECURITY_PROTOCOL_OTHER;
    } else {
        FFS_FAIL(FFS_ERROR);
    }

    return FFS_SUCCESS;
}
