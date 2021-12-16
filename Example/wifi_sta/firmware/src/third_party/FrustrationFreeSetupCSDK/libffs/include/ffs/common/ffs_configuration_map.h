/** @file ffs_configuration_map.h
 *
 * @brief Ffs configuration map types.
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

#ifndef FFS_CONFIGURATION_MAP_H_
#define FFS_CONFIGURATION_MAP_H_

#include "ffs/common/ffs_stream.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Map value types.
 */
typedef enum {
    FFS_MAP_VALUE_TYPE_BYTES, //!< Raw bytes value.
    FFS_MAP_VALUE_TYPE_STRING, //!< String value.
    FFS_MAP_VALUE_TYPE_INTEGER, //!< Integer value.
    FFS_MAP_VALUE_TYPE_BOOLEAN, //!< Boolean value.
} FFS_MAP_VALUE_TYPE;

/** @brief Configuration map value structure.
 */
typedef struct {
    FFS_MAP_VALUE_TYPE type; //!< Map value type.
    FfsStream_t bytesStream; //!< Raw bytes value stream.
    FfsStream_t stringStream; //!< String value stream.
    int32_t integerValue; //!< Integer value.
    bool booleanValue; //!< Boolean value.
} FfsMapValue_t;

/** @brief Configuration map entry structure.
 *
 * Structure encapsulating the deserialized configuration map key/value pair.
 */
typedef struct {
    FfsStream_t keyStream; //!< Key stream.
    FfsMapValue_t value; //!< Value.
} FfsConfigurationEntry_t;

extern const char *FFS_CONFIGURATION_ENTRY_KEY_COUNTRY_CODE; //!< Country code (\a e.g.: "US") key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_REALM; //!< Realm (\a e.g.: "USAmazon") key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_MARKETPLACE; //!< Obfuscated marketplace (\a e.g.: "ATVPDKIKX0DER") key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_LANGUAGE_LOCALE; //!< Language locale (\a e.g.: "en-US") key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_COUNTRY_OF_RESIDENCE; //!< Country of residence (\a e.g.: "US") key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_REGION; //!< Region (\a e.g.: "US") key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_REPORTING_URL; //!< (Device Setup Service) reporting URL key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_DSS_HOST; //!< Device Setup Service host key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_DSS_PORT; //!< Device Setup Service port key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_DSS_SESSION_TOKEN; //!< Final session token being passed.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_ALEXA_EVENT_GATEWAY_ENDPOINT; //!< Client SmartHome endpoint key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_UTC_TIME; //!< ISO8601 UTC time key.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_MANUFACTURER_NAME; //!< Manufacturer name, \a e.g., "Amazon".
extern const char *FFS_CONFIGURATION_ENTRY_KEY_MODEL_NUMBER; //!< Device model number, \a e.g., "A39GNED7NAJGKP".
extern const char *FFS_CONFIGURATION_ENTRY_KEY_SERIAL_NUMBER; //!< Device serial number, \a e.g., "G030JU0660540206".
extern const char *FFS_CONFIGURATION_ENTRY_KEY_HARDWARE_VERSION; //!< Device hardware revision, \a e.g., "0.0.0".
extern const char *FFS_CONFIGURATION_ENTRY_KEY_FIRMWARE_VERSION; //!< Device firmware revision, \a e.g., "0.6.195".
extern const char *FFS_CONFIGURATION_ENTRY_KEY_PIN; //!< Device PIN.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_CPU_ID; //!< Device CPU ID, \a e.g., "0000000b0029444e".
extern const char *FFS_CONFIGURATION_ENTRY_KEY_BLE_DEVICE_NAME; //!< BLE device name, \a e.g., "DashButton".
extern const char *FFS_CONFIGURATION_ENTRY_KEY_BLE_TRANSMIT_POWER; //!< BLE transmit power in the range -30dBm to +3dBm as a signed byte, \a e.g., -6dBm = 0xfa.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_WIFI_MAC_ADDRESS; //!< Wi-Fi MAC address, \a e.g., { 0x74, 0xc2, 0x46, 0xbb, 0x44, 0x41 }.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_PRODUCT_INDEX; //!< Product index, \a e.g., "CbtN".
extern const char *FFS_CONFIGURATION_ENTRY_KEY_SOFTWARE_VERSION_INDEX; //!< Software version index, \a e.g., "00".
extern const char *FFS_CONFIGURATION_ENTRY_KEY_DEVICE_EC_PUBLIC_KEY_DER; //!< Device public key in raw DER encoded bytes, \a It will be 256bit EC public key. When DER encoded it becomes 91 bytes.
extern const char *FFS_CONFIGURATION_ENTRY_KEY_CLOUD_EC_PUBLIC_KEY_DER; //!< Cloud service public key in raw DER encoded bytes, \a It will be 256bit EC public key. When DER encoded it becomes 91 bytes.

#define FFS_MAXIMUM_ISO8601_STRING_LENGTH (40) //!< Maximum ISO8601 time string length (sub-picosecond resolution).

#ifdef __cplusplus
}
#endif

#endif /* FFS_CONFIGURATION_MAP_H_ */
