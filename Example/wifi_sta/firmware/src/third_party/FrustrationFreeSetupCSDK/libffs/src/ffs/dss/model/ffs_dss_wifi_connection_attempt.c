/** @file ffs_dss_wifi_connection_attempt.c
 *
 * @brief DSS Wi-Fi connection attempt implementation.
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
#include "ffs/dss/model/ffs_dss_wifi_connection_attempt.h"

#define JSON_KEY_SSID                   "ssid"
#define JSON_KEY_SECURITY_PROTOCOL      "securityProtocol"
#define JSON_KEY_WIFI_CONNECTION_STATE  "wifiConnectionState"
#define JSON_KEY_ERROR_DETAILS          "errorDetails"

/*
 * Serialize DSS Wi-Fi connection attempt.
 */
FFS_RESULT ffsDssSerializeWifiConnectionAttempt(
        FfsDssWifiConnectionAttempt_t *wifiConnectionAttempt, FfsStream_t *outputStream)
{
    // Start the "Wi-Fi connection attempt" object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the SSID field.
    FFS_CHECK_RESULT(ffsEncodeJsonStreamField(JSON_KEY_SSID, &wifiConnectionAttempt->ssidStream,
            outputStream));

    // Serialize the security protocol field.
    const char *securityProtocolString;
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsDssGetWifiSecurityProtocolString(wifiConnectionAttempt->securityProtocol,
            &securityProtocolString));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_SECURITY_PROTOCOL, securityProtocolString,
            outputStream));

    // Serialize the connection state field.
    const char *connectionStateString;
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsDssGetWifiConnectionStateString(wifiConnectionAttempt->state,
            &connectionStateString));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_WIFI_CONNECTION_STATE, connectionStateString,
            outputStream));

    // Do we have error details?
    if (wifiConnectionAttempt->hasErrorDetails) {

        // Serialize the error details field.
        FFS_CHECK_RESULT(ffsDssSerializeErrorDetailsField(&wifiConnectionAttempt->errorDetails,
                outputStream));
    }

    // End the "Wi-Fi connection attempt" object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    return FFS_SUCCESS;
}

/*
 * Deserialize DSS Wi-Fi connection attempt.
 */
FFS_RESULT ffsDssDeserializeWifiConnectionAttempt(FfsJsonValue_t *wifiConnectionAttemptValue,
        FfsDssWifiConnectionAttempt_t *wifiConnectionAttempt)
{
    (void) wifiConnectionAttemptValue;
    (void) wifiConnectionAttempt;

    return FFS_NOT_IMPLEMENTED;
}

