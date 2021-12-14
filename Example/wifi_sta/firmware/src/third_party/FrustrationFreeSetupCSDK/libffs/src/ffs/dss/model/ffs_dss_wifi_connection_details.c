/** @file ffs_dss_wifi_connection_details.c
 *
 * @brief DSS Wi-Fi connection details implementation.
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
#include "ffs/dss/model/ffs_dss_wifi_connection_details.h"

#define JSON_KEY_SSID                   "ssid"
#define JSON_KEY_SECURITY_PROTOCOL      "securityProtocol"
#define JSON_KEY_WIFI_CONNECTION_STATE       "wifiConnectionState"
#define JSON_KEY_ERROR_DETAILS          "errorDetails"

/*
 * Serialize DSS Wi-Fi connection details.
 */
FFS_RESULT ffsDssSerializeWifiConnectionDetails(
        FfsDssWifiConnectionDetails_t *wifiConnectionDetails, FfsStream_t *outputStream)
{
    // Start the "Wi-Fi connection details" object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectStart(outputStream));

    // Serialize the SSID field.
    FFS_CHECK_RESULT(ffsEncodeJsonQuotedStreamField(JSON_KEY_SSID, &wifiConnectionDetails->ssidStream,
            outputStream));

    // Serialize the security protocol field.
    const char *securityProtocolString;
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsDssGetWifiSecurityProtocolString(wifiConnectionDetails->securityProtocol,
            &securityProtocolString));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_SECURITY_PROTOCOL, securityProtocolString,
            outputStream));

    // Serialize the connection state field.
    const char *connectionStateString;
    FFS_CHECK_RESULT(ffsEncodeJsonSeparator(outputStream));
    FFS_CHECK_RESULT(ffsDssGetWifiConnectionStateString(wifiConnectionDetails->state,
            &connectionStateString));
    FFS_CHECK_RESULT(ffsEncodeJsonStringField(JSON_KEY_WIFI_CONNECTION_STATE, connectionStateString,
            outputStream));

    // Do we have error details?
    if (wifiConnectionDetails->hasErrorDetails) {

        // Serialize the error details field.
        FFS_CHECK_RESULT(ffsDssSerializeErrorDetailsField(&wifiConnectionDetails->errorDetails,
                outputStream));
    }

    // End the "Wi-Fi connection details" object.
    FFS_CHECK_RESULT(ffsEncodeJsonObjectEnd(outputStream));

    return FFS_SUCCESS;
}

/*
 * Deserialize DSS Wi-Fi connection details.
 */
FFS_RESULT ffsDssDeserializeWifiConnectionDetails(FfsJsonValue_t *wifiConnectionDetailsValue,
        FfsDssWifiConnectionDetails_t *wifiConnectionDetails)
{
    (void) wifiConnectionDetailsValue;
    (void) wifiConnectionDetails;

    return FFS_NOT_IMPLEMENTED;
}
