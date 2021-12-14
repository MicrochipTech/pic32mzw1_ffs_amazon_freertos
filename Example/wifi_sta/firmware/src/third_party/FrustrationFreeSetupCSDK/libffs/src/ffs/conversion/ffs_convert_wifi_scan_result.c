/** @file ffs_convert_wifi_credentials.h
 *
 * @brief Convert between API and DSS Wi-Fi scan results implementation.
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
#include "ffs/conversion/ffs_convert_wifi_scan_result.h"
#include "ffs/conversion/ffs_convert_wifi_security_protocol.h"

/*
 * Convert an API Wi-Fi scan result to a DSS Wi-Fi scan result.
 */
FFS_RESULT ffsConvertApiWifiScanResultToDss(FfsWifiScanResult_t *apiWifiScanResult,
        FfsDssWifiScanResult_t *dssWifiScanResult)
{
    // Copy the fields with the same type.
    dssWifiScanResult->bssidStream = apiWifiScanResult->bssidStream;
    dssWifiScanResult->ssidStream = apiWifiScanResult->ssidStream;
    dssWifiScanResult->frequencyBand = apiWifiScanResult->frequencyBand;
    dssWifiScanResult->signalStrength = apiWifiScanResult->signalStrength;

    // Convert the security protocol.
    FFS_CHECK_RESULT(ffsConvertApiWifiSecurityProtocolToDss(apiWifiScanResult->securityProtocol,
            &dssWifiScanResult->securityProtocol));

    return FFS_SUCCESS;
}

/*
 * Convert a DSS Wi-Fi scan result to an API Wi-Fi scan result.
 */
FFS_RESULT ffsConvertDssWifiScanResultToApi(FfsDssWifiScanResult_t *dssWifiScanResult,
        FfsWifiScanResult_t *apiWifiScanResult)
{
    (void) dssWifiScanResult;
    (void) apiWifiScanResult;

    return FFS_NOT_IMPLEMENTED;
}
