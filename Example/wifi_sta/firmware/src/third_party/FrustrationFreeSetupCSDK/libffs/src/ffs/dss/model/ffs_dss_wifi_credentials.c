/** @file ffs_dss_wifi_credentials.c
 *
 * @brief DSS Wi-Fi credentials serialization/deserialization implementation.
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
#include "ffs/common/ffs_hex.h"
#include "ffs/dss/model/ffs_dss_wifi_credentials.h"

#define JSON_KEY_SSID                   "ssid"
#define JSON_KEY_SECURITY_PROTOCOL      "securityProtocol"
#define JSON_KEY_KEY                    "key"
#define JSON_KEY_KEY_INDEX              "keyIndex"
#define JSON_KEY_PRIORITY               "priority"
#define JSON_KEY_FREQUENCY              "frequency"

#define WEP_64_HEX_LENGTH               (10)
#define WEP_128_HEX_LENGTH              (26)

// Static function prototypes.
static bool ffsUtf8WepKeyIsQuoted(FfsStream_t wepKeyStream);
static FFS_RESULT ffsGetInnerUtf8WepKey(FfsStream_t wepKeyStream, FfsStream_t *destinationStream);

/*
 * Serialize DSS Wi-Fi credentials.
 */
FFS_RESULT ffsDssSerializeWifiCredentials(FfsDssWifiCredentials_t *wifiCredentials,
        FfsStream_t *outputStream)
{
    (void) wifiCredentials;
    (void) outputStream;

    return FFS_NOT_IMPLEMENTED;
}

/*
 * Deserialize DSS Wi-Fi credentials.
 */
FFS_RESULT ffsDssDeserializeWifiCredentials(FfsJsonValue_t *wifiCredentialsValue,
        FfsDssWifiCredentials_t *wifiCredentials)
{
    // Zero out the Wi-Fi credentials object.
    memset(wifiCredentials, 0, sizeof(*wifiCredentials));

    // Parse {"ssid":"\"...\"","securityProtocol"":"...","key":"...","keyIndex":...,"priority":...,"frequency":...}.
    FfsJsonField_t ssidField = ffsCreateJsonField(JSON_KEY_SSID, FFS_JSON_STRING);
    FfsJsonField_t securityProtocolField = ffsCreateJsonField(JSON_KEY_SECURITY_PROTOCOL,
            FFS_JSON_STRING);
    FfsJsonField_t keyField = ffsCreateJsonField(JSON_KEY_KEY, FFS_JSON_ANY);
    FfsJsonField_t keyIndexField = ffsCreateJsonField(JSON_KEY_KEY_INDEX, FFS_JSON_NUMBER);
    FfsJsonField_t priorityField = ffsCreateJsonField(JSON_KEY_PRIORITY, FFS_JSON_NUMBER);
    FfsJsonField_t frequencyField = ffsCreateJsonField(JSON_KEY_FREQUENCY, FFS_JSON_NUMBER);
    FfsJsonField_t *wifiCredentialsExpectedFields[] =
            { &ssidField, &securityProtocolField, &keyField, &keyIndexField, &priorityField, &frequencyField, NULL };

    FFS_CHECK_RESULT(ffsParseJsonObject(wifiCredentialsValue, wifiCredentialsExpectedFields));

    // Parse the SSID field, stripping additional quotes (reusing the JSON buffer).
    FFS_CHECK_RESULT(ffsParseJsonQuotedString(&ssidField.value, &wifiCredentials->ssidStream));

    // Parse the security protocol string.
    const char *securityProtocolString;
    FFS_CHECK_RESULT(ffsConvertJsonFieldToUtf8String(&securityProtocolField, &securityProtocolString));

    // Convert to the enumerated value.
    FFS_CHECK_RESULT(ffsDssParseWifiSecurityProtocol(securityProtocolString,
            &wifiCredentials->securityProtocol));

    // Parse protocol-dependent values.
    if (wifiCredentials->securityProtocol == FFS_DSS_WIFI_SECURITY_PROTOCOL_WPA_PSK) {

        // Parse the PSK for a WPA/PSK network.
        FFS_CHECK_RESULT(ffsParseJsonQuotedString(&keyField.value, &wifiCredentials->keyStream));
    } else if (wifiCredentials->securityProtocol == FFS_DSS_WIFI_SECURITY_PROTOCOL_WEP) {

        // Parse the key for a WEP network.
        FFS_CHECK_RESULT(ffsConvertJsonValueToUtf8(&keyField.value, &wifiCredentials->keyStream));

        // Strip quotes, in they are present.
        FFS_CHECK_RESULT(ffsGetInnerUtf8WepKey(wifiCredentials->keyStream, &wifiCredentials->keyStream));

        // Convert to ASCII, if hex.
        if (ffsStreamIsHex(wifiCredentials->keyStream) && (FFS_STREAM_DATA_SIZE(wifiCredentials->keyStream) == WEP_64_HEX_LENGTH
                || FFS_STREAM_DATA_SIZE(wifiCredentials->keyStream) == WEP_128_HEX_LENGTH)) {
            FfsStream_t hexStream = wifiCredentials->keyStream;
            FFS_CHECK_RESULT(ffsFlushStream(&wifiCredentials->keyStream));
            FFS_CHECK_RESULT(ffsParseHexStream(&hexStream, &wifiCredentials->keyStream));
        }

        // Parse the key index?
        if (!ffsJsonFieldIsEmpty(&keyIndexField)) {
            FFS_CHECK_RESULT(ffsParseJsonInt32(&keyIndexField.value, &wifiCredentials->wepIndex));
        }
    }

    // Parse the network priority?
    if (!ffsJsonFieldIsEmpty(&priorityField)) {
        FFS_CHECK_RESULT(ffsParseJsonInt32(&priorityField.value, &wifiCredentials->networkPriority));
    }

    // Parse the network frequency?
    if (!ffsJsonFieldIsEmpty(&frequencyField)) {
        FFS_CHECK_RESULT(ffsParseJsonInt32(&frequencyField.value, &wifiCredentials->frequency));
    }

    return FFS_SUCCESS;
}

static bool ffsUtf8WepKeyIsQuoted(FfsStream_t wepKeyStream) {

    // Assert stream has at least two characters.
    if (FFS_STREAM_DATA_SIZE(wepKeyStream) < 2) {
        return false;
    }

    // Read the first quote.
    if (ffsReadExpected(&wepKeyStream, "\"") != FFS_SUCCESS) {
        return false;
    }

    // Read to the last character.
    while (FFS_STREAM_DATA_SIZE(wepKeyStream) > 1) {
        if (ffsReadStream(&wepKeyStream, 1, NULL) != FFS_SUCCESS) {
            return false;
        }
    }

    // Read the second quote.
    if (ffsReadExpected(&wepKeyStream, "\"") != FFS_SUCCESS) {
        return false;
    }

    return true;
}

static FFS_RESULT ffsGetInnerUtf8WepKey(FfsStream_t wepKeyStream, FfsStream_t *destinationStream) {

    // Assert WEP key is quoted.
    if (!ffsUtf8WepKeyIsQuoted(wepKeyStream)) {
        *destinationStream = wepKeyStream;
        return FFS_SUCCESS;
    }

    // Read the first quote.
    FFS_CHECK_RESULT(ffsReadExpected(&wepKeyStream, "\""));

    // Get the inner data.
    *destinationStream = ffsCreateInputStream(FFS_STREAM_NEXT_READ(wepKeyStream), FFS_STREAM_DATA_SIZE(wepKeyStream) - 1);

    return FFS_SUCCESS;
}
