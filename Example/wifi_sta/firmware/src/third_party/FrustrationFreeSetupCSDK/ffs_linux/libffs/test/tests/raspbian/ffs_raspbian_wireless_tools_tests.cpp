/** @file ffs_wireless_tools_tests.cpp
 *
 * @copyright 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 * AMAZON PROPRIETARY/CONFIDENTIAL
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 * This file is a Modifiable File, as defined in the accompanying LICENSE.TXT
 * file.
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "ffs/linux/ffs_shell.h"
#include <gmock/gmock.h>
#include <ffs/raspbian/ffs_raspbian_wireless_tools.h>

#define WIFI_INTERFACE             "wlan0"
#define WEP_SSID                   "WepSSID"
#define WEP_INVALID_FORMAT_KEY     "key"

#if defined(__APPLE__) || defined(BRAZIL)
TEST(WirelessToolsTests, DISABLED_TestConnectWithWirelessToolsInvalidFormatKey)
#else
TEST(WirelessToolsTests, TestConnectWithWirelessToolsInvalidFormatKey)
#endif
{
    FfsLinuxWifiContext_t wifiContext;
    wifiContext.interface = WIFI_INTERFACE;
    wifiContext.connectionDetails.state = FFS_WIFI_CONNECTION_STATE_IDLE;

    FfsWifiConfiguration_t configuration;
    configuration.ssidStream = FFS_STRING_INPUT_STREAM(WEP_SSID);
    configuration.keyStream = FFS_STRING_INPUT_STREAM(WEP_INVALID_FORMAT_KEY);
    configuration.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WEP;

    ASSERT_EQ(ffsRaspbianConnectToWifi(&wifiContext, &configuration), FFS_SUCCESS);

    ASSERT_EQ(wifiContext.connectionDetails.state, FFS_WIFI_CONNECTION_STATE_FAILED);
}
