/** @file ffs_dss_wifi_scan_result.c
 *
 * @brief DSS Wi-Fi scan result serialization/deserialization implementation.
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
#include "ffs/dss/model/ffs_dss_wifi_scan_result.h"

#include <stdio.h>

#define JSON_KEY_SSID                   "ssid"
#define JSON_KEY_BSSID                  "bssid"
#define JSON_KEY_SECURITY_PROTOCOL      "securityProtocol"
#define JSON_KEY_RSSI                   "rssi"
#define JSON_KEY_FREQUENCY              "frequency"

#define BSSID_SIZE                      (6)
#define BSSID_STRING_SIZE               (3 * BSSID_SIZE)
#define BSSID_STRING_FORMAT             "%02X:%02X:%02X:%02X:%02X:%02X"

/*
 * Serialize a DSS Wi-Fi scan result.
 */
FFS_RESULT ffsDssSerializeWifiScanResult(FfsDssWifiScanResult_t *wifiScanResult,
        FfsStream_t *outputStream)
{
    // Start the Wi-Fi scan result object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the SSID (required).
    FFS_CHECK_RESULT(ffsEncodeJsonQuotedStreamField(JSON_KEY_SSID, &wifiScanResult->ssidStream, outputStream));

    // Serialize the BSSID?
    if (!ffsStreamIsEmpty(&wifiScanResult->bssidStream)) {
        FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));

        // Read the BSSID bytes.
        uint8_t *bssidData;
        FFS_CHECK_RESULT(ffsReadStream(&wifiScanResult->bssidStream, BSSID_SIZE, &bssidData));

        // Convert it to a string.
        char bssidString[BSSID_STRING_SIZE];
        snprintf(bssidString, sizeof(bssidString), BSSID_STRING_FORMAT,
                bssidData[0], bssidData[1], bssidData[2], bssidData[3], bssidData[4], bssidData[5]);

        // Serialize it.
        FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_BSSID, bssidString, outputStream));
    }

    // Serialize the security protocol.
    const char *securityProtocolString;
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsDssGetWifiSecurityProtocolString(wifiScanResult->securityProtocol,
            &securityProtocolString));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_SECURITY_PROTOCOL, securityProtocolString,
            outputStream));

    // Serialize the RSSI?
    if (wifiScanResult->signalStrength) {
        FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
        FFS_CHECK_RESULT(ffsEncodeJsonInt32Field(JSON_KEY_RSSI, wifiScanResult->signalStrength,
                outputStream));
    }

    // Serialize the channel frequency?
    if (wifiScanResult->frequencyBand) {
        FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
        FFS_CHECK_RESULT(ffsEncodeJsonInt32Field(JSON_KEY_FREQUENCY, wifiScanResult->frequencyBand,
                outputStream));
    }

    // End the Wi-Fi scan result object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    return FFS_SUCCESS;
}

/*
 * Deserialize a DSS Wi-Fi scan result.
 */
FFS_RESULT ffsDssDeserializeWifiScanResult(FfsJsonValue_t *wifiScanResultValue,
        FfsDssWifiScanResult_t *wifiScanResult)
{
    (void) wifiScanResultValue;
    (void) wifiScanResult;

    return FFS_NOT_IMPLEMENTED;
}
