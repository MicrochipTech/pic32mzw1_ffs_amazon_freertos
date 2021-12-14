/** @file ffs_configuration_map.c
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

#include "ffs/common/ffs_configuration_map.h"

const char *FFS_CONFIGURATION_ENTRY_KEY_COUNTRY_CODE =                 "LocaleConfiguration.CountryCode"; //!< Country code (\a e.g.: "US") key.
const char *FFS_CONFIGURATION_ENTRY_KEY_REALM =                        "LocaleConfiguration.Realm"; //!< Realm (\a e.g.: "USAmazon") key.
const char *FFS_CONFIGURATION_ENTRY_KEY_MARKETPLACE =                  "LocaleConfiguration.Marketplace"; //!< Obfuscated marketplace (\a e.g.: "ATVPDKIKX0DER") key.
const char *FFS_CONFIGURATION_ENTRY_KEY_LANGUAGE_LOCALE =              "LocaleConfiguration.LanguageLocale"; //!< Language locale (\a e.g.: "en-US") key.
const char *FFS_CONFIGURATION_ENTRY_KEY_COUNTRY_OF_RESIDENCE =         "LocaleConfiguration.CountryOfResidence"; //!< Country of residence (\a e.g.: "US") key.
const char *FFS_CONFIGURATION_ENTRY_KEY_REGION =                       "LocaleConfiguration.Region"; //!< Region (\a e.g.: "US") key.
const char *FFS_CONFIGURATION_ENTRY_KEY_REPORTING_URL =                "DSS.ReportUrl"; //!< (Device Setup Service) reporting URL key.
const char *FFS_CONFIGURATION_ENTRY_KEY_DSS_HOST =                     "DSS.Host"; //!< Device Setup Service host key.
const char *FFS_CONFIGURATION_ENTRY_KEY_DSS_PORT =                     "DSS.Port"; //!< Device Setup Service port key.
const char *FFS_CONFIGURATION_ENTRY_KEY_ALEXA_EVENT_GATEWAY_ENDPOINT = "SmartHome.AlexaEventGatewayEndpoint"; //!< Client SmartHome endpoint key.
const char *FFS_CONFIGURATION_ENTRY_KEY_DSS_SESSION_TOKEN =            "FFS.SessionToken"; //!< Final session token being passed.
const char *FFS_CONFIGURATION_ENTRY_KEY_UTC_TIME =                     "Time.UTC"; //!< ISO8601 UTC Time key.
const char *FFS_CONFIGURATION_ENTRY_KEY_MANUFACTURER_NAME =            "DeviceInformation.ManufacturerName"; //!< Manufacturer name, \a e.g., "Amazon".
const char *FFS_CONFIGURATION_ENTRY_KEY_MODEL_NUMBER =                 "DeviceInformation.ModelNumber"; //!< Device model number, \a e.g., "A39GNED7NAJGKP".
const char *FFS_CONFIGURATION_ENTRY_KEY_SERIAL_NUMBER =                "DeviceInformation.SerialNumber"; //!< Device serial number, \a e.g., "G030JU0660540206".
const char *FFS_CONFIGURATION_ENTRY_KEY_HARDWARE_VERSION =             "DeviceInformation.HardwareVersion"; //!< Device hardware revision, \a e.g., "0.0.0".
const char *FFS_CONFIGURATION_ENTRY_KEY_FIRMWARE_VERSION =             "DeviceInformation.FirmwareVersion"; //!< Device firmware revision, \a e.g., "0.6.195".
const char *FFS_CONFIGURATION_ENTRY_KEY_PIN =                          "DeviceInformation.Pin"; //!< Device PIN.
const char *FFS_CONFIGURATION_ENTRY_KEY_CPU_ID =                       "DeviceInformation.CpuId"; //!< Device CPU ID, \a e.g., "0000000b0029444e".
const char *FFS_CONFIGURATION_ENTRY_KEY_BLE_DEVICE_NAME =              "DeviceInformation.BleDeviceName"; //!< BLE device name, \a e.g., "DashButton".
const char *FFS_CONFIGURATION_ENTRY_KEY_BLE_TRANSMIT_POWER =           "DeviceInformation.BleTransmitPower"; //!< BLE transmit power in the range -30dBm to +3dBm as a signed byte, \a e.g., -6dBm = 0xfa.
const char *FFS_CONFIGURATION_ENTRY_KEY_WIFI_MAC_ADDRESS =             "DeviceInformation.WifiMacAddress"; //!< Wi-Fi MAC address, \a e.g., { 0x74, 0xc2, 0x46, 0xbb, 0x44, 0x41 }.
const char *FFS_CONFIGURATION_ENTRY_KEY_PRODUCT_INDEX =                "DeviceInformation.ProductIndex"; //!< Product index, \a e.g., "CbtN".
const char *FFS_CONFIGURATION_ENTRY_KEY_SOFTWARE_VERSION_INDEX =       "DeviceInformation.SoftwareVersionIndex"; //!< Software version index, \a e.g., "00".
const char *FFS_CONFIGURATION_ENTRY_KEY_DEVICE_EC_PUBLIC_KEY_DER =        "DeviceInformation.PublicKey"; //!< Device public key in raw DER encoded bytes, \a It will be 256bit EC public key. When DER encoded it becomes 91 bytes.
const char *FFS_CONFIGURATION_ENTRY_KEY_CLOUD_EC_PUBLIC_KEY_DER =         "DSS.PublicKey"; //!< Cloud service public key in raw DER encoded bytes, \a It will be 256bit EC public key. When DER encoded it becomes 91 bytes.
