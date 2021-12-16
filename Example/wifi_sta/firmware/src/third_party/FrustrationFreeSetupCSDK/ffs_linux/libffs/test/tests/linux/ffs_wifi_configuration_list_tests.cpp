/** @file ffs_wifi_configuration_list_tests.cpp
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

#include "ffs/compat/ffs_linux_user_context.h"
#include "ffs/linux/ffs_wifi_configuration_list.h"
#include "ffs/linux/ffs_wifi_context.h"

#include <gmock/gmock.h>

#define SSID1       "SSID1"
#define SSID2       "SSID2"
#define PASSWORD1   "PASSWORD1"
#define PASSWORD2   "PASSWORD2"
#define WEP_INDEX   1
#define IS_HIDDEN   true
#define PRIORTIY    3

#define ZERO_FILL(variable) memset(&variable, 0, sizeof(variable))

static bool areWifiConfigurationsEqual(FfsWifiConfiguration_t *configuration1,
    FfsWifiConfiguration_t *configuration2);

TEST(WifiConfigurationListTests, Push)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiConfiguration_t configuration;
    configuration.ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    configuration.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
    ASSERT_EQ(ffsSetStreamToNull(&configuration.keyStream), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiConfigurationListPush(&wifiContext, &configuration), FFS_SUCCESS);

    return;
}

TEST(WifiConfigurationListTests, PeekEmpty)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiConfiguration_t *configuration;

    ASSERT_EQ(ffsWifiConfigurationListPeek(&wifiContext, &configuration), FFS_ERROR);

    return;
}

TEST(WifiConfigurationListTests, Peek)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiConfiguration_t *configuration;
    FfsWifiConfiguration_t configurationList[2];

    configurationList[0].ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    configurationList[0].keyStream = FFS_STRING_INPUT_STREAM(PASSWORD1);
    configurationList[0].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    ASSERT_EQ(ffsWifiConfigurationListPush(&wifiContext, &configurationList[0]), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiConfigurationListPeek(&wifiContext, &configuration), FFS_SUCCESS);

    ASSERT_TRUE(areWifiConfigurationsEqual(configuration, &configurationList[0]));

    configurationList[1].ssidStream = FFS_STRING_INPUT_STREAM(SSID2);
    configurationList[1].keyStream = FFS_STRING_INPUT_STREAM(PASSWORD2);
    configurationList[1].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WEP;
    configurationList[1].wepIndex = WEP_INDEX;

    ASSERT_EQ(ffsWifiConfigurationListPush(&wifiContext, &configurationList[1]), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiConfigurationListPeek(&wifiContext, &configuration), FFS_SUCCESS);

    ASSERT_TRUE(areWifiConfigurationsEqual(configuration, &configurationList[0]));

    return;
}

TEST(WifiConfigurationListTests, PopEmpty)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiConfigurationListPop(&wifiContext), FFS_ERROR);

    return;
}

TEST(WifiConfigurationListTests, Pop)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiConfiguration_t *configuration;
    FfsWifiConfiguration_t configurationList[2];

    configurationList[0].ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    configurationList[0].keyStream = FFS_STRING_INPUT_STREAM(PASSWORD1);
    configurationList[0].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    ASSERT_EQ(ffsWifiConfigurationListPush(&wifiContext, &configurationList[0]), FFS_SUCCESS);

    configurationList[1].ssidStream = FFS_STRING_INPUT_STREAM(SSID2);
    configurationList[1].keyStream = FFS_STRING_INPUT_STREAM(PASSWORD2);
    configurationList[1].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WEP;
    configurationList[1].wepIndex = WEP_INDEX;

    ASSERT_EQ(ffsWifiConfigurationListPush(&wifiContext, &configurationList[1]), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiConfigurationListPop(&wifiContext), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiConfigurationListPeek(&wifiContext, &configuration), FFS_SUCCESS);

    ASSERT_TRUE(areWifiConfigurationsEqual(configuration, &configurationList[1]));

    return;
}

TEST(WifiConfigurationListTests, Clear)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiConfiguration_t configurationList[2];

    configurationList[0].ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    configurationList[0].keyStream = FFS_STRING_INPUT_STREAM(PASSWORD1);
    configurationList[0].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    ASSERT_EQ(ffsWifiConfigurationListPush(&wifiContext, &configurationList[0]), FFS_SUCCESS);

    configurationList[1].ssidStream = FFS_STRING_INPUT_STREAM(SSID2);
    configurationList[1].keyStream = FFS_STRING_INPUT_STREAM(PASSWORD2);
    configurationList[1].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WEP;
    configurationList[1].wepIndex = WEP_INDEX;

    ASSERT_EQ(ffsWifiConfigurationListPush(&wifiContext, &configurationList[1]), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiConfigurationListClear(&wifiContext), FFS_SUCCESS);

    bool isEmpty;
    ASSERT_EQ(ffsWifiConfigurationListIsEmpty(&wifiContext, &isEmpty), FFS_SUCCESS);
    ASSERT_TRUE(isEmpty);

    return;
}

TEST(WifiConfigurationListTests, IsEmpty)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    bool isEmpty;
    FfsWifiConfiguration_t configuration;

    ASSERT_EQ(ffsWifiConfigurationListIsEmpty(&wifiContext, &isEmpty), FFS_SUCCESS);
    ASSERT_TRUE(isEmpty);

    configuration.ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    configuration.keyStream = FFS_STRING_INPUT_STREAM(PASSWORD1);
    configuration.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    ASSERT_EQ(ffsWifiConfigurationListPush(&wifiContext, &configuration), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiConfigurationListIsEmpty(&wifiContext, &isEmpty), FFS_SUCCESS);
    ASSERT_FALSE(isEmpty);

    ASSERT_EQ(ffsWifiConfigurationListPop(&wifiContext), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiConfigurationListIsEmpty(&wifiContext, &isEmpty), FFS_SUCCESS);
    ASSERT_TRUE(isEmpty);

    return;
}

static bool areWifiConfigurationsEqual(FfsWifiConfiguration_t *configuration1,
    FfsWifiConfiguration_t *configuration2)
{
    bool equal = true;

    ffsRewindStream(&configuration1->ssidStream);
    ffsRewindStream(&configuration1->keyStream);

    ffsRewindStream(&configuration2->ssidStream);
    ffsRewindStream(&configuration2->keyStream);

    equal &= ffsStreamMatchesStream(&configuration1->ssidStream, &configuration2->ssidStream);
    equal &= ffsStreamMatchesStream(&configuration1->keyStream, &configuration2->keyStream);
    equal &= (configuration1->isHiddenNetwork == configuration2->isHiddenNetwork);
    equal &= (configuration1->networkPriority == configuration2->networkPriority);
    equal &= (configuration1->wepIndex == configuration2->wepIndex);
    equal &= (configuration1->securityProtocol == configuration2->securityProtocol);

    return equal;
}
