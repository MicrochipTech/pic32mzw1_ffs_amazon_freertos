/** @file ffs_wifi.h
 *
 * @brief Wi-Fi-related types.
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

#ifndef FFS_WIFI_H_
#define FFS_WIFI_H_

#include "ffs/common/ffs_error_details.h"
#include "ffs/common/ffs_json.h"
#include "ffs/common/ffs_result.h"
#include "ffs/common/ffs_stream.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FFS_BSSID_SIZE (6) //!< Binary SSID size.
#define FFS_MAXIMUM_SSID_SIZE (32) //!< Maximum SSID size.
#define FFS_MAXIMUM_WIFI_KEY_SIZE (64) //!< Maximum unencoded key size.
#define FFS_MAXIMUM_WIFI_SCAN_RESULT_SIZE (86 + \
        FFS_MAXIMUM_SSID_SIZE * FFS_JSON_MAXIMUM_ENCODED_CHARACTER_SIZE) //!< Maximum JSON-format Wi-Fi scan result (null is 6 characters in JSON).

/** @brief Wi-Fi security protocol enumeration.
 */
typedef enum {
    FFS_WIFI_SECURITY_PROTOCOL_NONE = 0, //!< Open network.
    FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK = 1, //!< WPA.
    FFS_WIFI_SECURITY_PROTOCOL_WEP = 2, //!< WEP.
    FFS_WIFI_SECURITY_PROTOCOL_OTHER = 3, //!< Other protocol.
    FFS_WIFI_SECURITY_PROTOCOL_UNKNOWN = 4 //!< Unknown protocol.
} FFS_WIFI_SECURITY_PROTOCOL;

/** @brief Visible Wi-Fi network data structure.
 */
typedef struct {
    FfsStream_t ssidStream; //!< SSID.
    FfsStream_t bssidStream; //!< BSSID.
    FFS_WIFI_SECURITY_PROTOCOL securityProtocol; //!< Network security type.
    int32_t frequencyBand; //!< Frequency band.
    int32_t signalStrength; //!< Relative received signal strength in dB.
} FfsWifiScanResult_t;

/** @brief Wi-Fi configuration structure.
 */
typedef struct {
    FfsStream_t ssidStream; //!< SSID.
    FFS_WIFI_SECURITY_PROTOCOL securityProtocol; //!< Network security type.
    int32_t networkPriority; //!< Network priority. @deprecated
    bool isHiddenNetwork; //!< Is the SSID hidden?
    int32_t wepIndex; //!< WEP index. @deprecated
    FfsStream_t keyStream; //!< WEP/WPA key.
} FfsWifiConfiguration_t;

/** @brief Wi-Fi connection state.
 */
typedef enum {
    FFS_WIFI_CONNECTION_STATE_IDLE = 0, //!< Initial Wi-Fi connection state.
    FFS_WIFI_CONNECTION_STATE_DISCONNECTED = 2, //!< Disconnected from Wi-Fi.
    FFS_WIFI_CONNECTION_STATE_UNAUTHENTICATED = 3, //!< Initial state of a Wi-Fi connection attempt.
    FFS_WIFI_CONNECTION_STATE_AUTHENTICATED = 4, //!< Authenticated with a Wi-Fi network.
    FFS_WIFI_CONNECTION_STATE_ASSOCIATED = 5, //!< Associated with (connected to) a Wi-Fi network.
    FFS_WIFI_CONNECTION_STATE_FAILED = -1 //!< Wi-Fi connection failed.
} FFS_WIFI_CONNECTION_STATE;

/** @brief Wi-Fi connection details.
 */
typedef struct {
    FfsStream_t ssidStream; //!< SSID.
    FFS_WIFI_SECURITY_PROTOCOL securityProtocol; //!< Network security type.
    FFS_WIFI_CONNECTION_STATE state; //!< Wi-Fi connection state.
    bool hasErrorDetails; //!< Has an error details field.
    FfsErrorDetails_t errorDetails; //!< Optional error details.
} FfsWifiConnectionDetails_t;

/** @brief Wi-Fi connection attempt details.
 *
 * This structure is used to store information about connection attempts
 * for later reporting.
 */
typedef FfsWifiConnectionDetails_t FfsWifiConnectionAttempt_t;

/** @brief Map Wi-Fi key management enumeration to security protocol string.
 *
 * @param securityProtocol The Wi-Fi security protocol
 * @param securityProtocolString The security protocol string
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetWifiSecurityProtocolString(FFS_WIFI_SECURITY_PROTOCOL securityProtocol,
        const char **securityProtocolString);

/** @brief Map Wi-Fi connection state to connection state string.
 *
 * @param wifiConnectionState The Wi-Fi connection state
 * @param connectionStateString The connection state string
 *
 * @returns Enumerated [result](@ref FFS_RESULT)
 */
FFS_RESULT ffsGetWifiConnectionStateString(FFS_WIFI_CONNECTION_STATE wifiConnectionState,
        const char **connectionStateString);

#ifdef __cplusplus
}
#endif

#endif /* FFS_WIFI_H_ */
