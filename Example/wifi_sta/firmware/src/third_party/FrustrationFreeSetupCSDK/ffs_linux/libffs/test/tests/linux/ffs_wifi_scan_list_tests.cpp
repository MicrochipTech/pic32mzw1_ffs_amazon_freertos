/** @file ffs_wifi_scan_result_list_tests.cpp
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
#include "ffs/linux/ffs_wifi_scan_list.h"
#include "ffs/linux/ffs_wifi_context.h"

#include <gmock/gmock.h>

#define SSID1       "SSID1"
#define SSID2       "SSID2"
#define BSSID1      "BSSID1"
#define BSSID2      "BSSID2"
#define PASSWORD1   "PASSWORD1"
#define PASSWORD2   "PASSWORD2"
#define WEP_INDEX   1
#define IS_HIDDEN   true
#define PRIORTIY    3

#define ZERO_FILL(variable) memset(&variable, 0, sizeof(variable))

static bool areWifiScanResultsEqual(FfsWifiScanResult_t *scanResult1,
    FfsWifiScanResult_t *scanResult2);

TEST(WifiScanListTests, Push)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiScanResult_t scanResult;
    scanResult.ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    scanResult.bssidStream = FFS_STRING_INPUT_STREAM(BSSID1);
    scanResult.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;

    ASSERT_EQ(ffsWifiScanListPush(&wifiContext, &scanResult), FFS_SUCCESS);

    return;
}

TEST(WifiScanListTests, PeekEmpty)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiScanResult_t *scanResult;

    ASSERT_EQ(ffsWifiScanListPeek(&wifiContext, &scanResult), FFS_ERROR);

    return;
}

TEST(WifiScanListTests, Peek)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiScanResult_t *scanResult;
    FfsWifiScanResult_t scanResultList[2];

    scanResultList[0].ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    scanResultList[0].bssidStream = FFS_STRING_INPUT_STREAM(BSSID1);
    scanResultList[0].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    ASSERT_EQ(ffsWifiScanListPush(&wifiContext, &scanResultList[0]), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiScanListPeek(&wifiContext, &scanResult), FFS_SUCCESS);

    ASSERT_TRUE(areWifiScanResultsEqual(scanResult, &scanResultList[0]));

    scanResultList[1].ssidStream = FFS_STRING_INPUT_STREAM(SSID2);
    scanResultList[1].bssidStream = FFS_STRING_INPUT_STREAM(BSSID2);
    scanResultList[1].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WEP;

    ASSERT_EQ(ffsWifiScanListPush(&wifiContext, &scanResultList[1]), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiScanListPeek(&wifiContext, &scanResult), FFS_SUCCESS);

    ASSERT_TRUE(areWifiScanResultsEqual(scanResult, &scanResultList[0]));

    return;
}

TEST(WifiScanListTests, PopEmpty)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiScanListPop(&wifiContext), FFS_ERROR);

    return;
}

TEST(WifiScanListTests, Pop)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiScanResult_t *scanResult;
    FfsWifiScanResult_t scanResultList[2];

    scanResultList[0].ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    scanResultList[0].bssidStream = FFS_STRING_INPUT_STREAM(BSSID1);
    scanResultList[0].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    ASSERT_EQ(ffsWifiScanListPush(&wifiContext, &scanResultList[0]), FFS_SUCCESS);

    scanResultList[1].ssidStream = FFS_STRING_INPUT_STREAM(SSID2);
    scanResultList[1].bssidStream = FFS_STRING_INPUT_STREAM(BSSID2);
    scanResultList[1].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WEP;

    ASSERT_EQ(ffsWifiScanListPush(&wifiContext, &scanResultList[1]), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiScanListPop(&wifiContext), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiScanListPeek(&wifiContext, &scanResult), FFS_SUCCESS);

    ASSERT_TRUE(areWifiScanResultsEqual(scanResult, &scanResultList[1]));

    return;
}

TEST(WifiScanListTests, HasNetwork)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiScanResult_t scanResult;
    scanResult.ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    scanResult.bssidStream = FFS_STRING_INPUT_STREAM(BSSID1);
    scanResult.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;
    ASSERT_EQ(ffsWifiScanListPush(&wifiContext, &scanResult), FFS_SUCCESS);

    FfsWifiConfiguration_t configuration1;
    configuration1.ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    configuration1.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_NONE;

    FfsWifiConfiguration_t configuration2;
    configuration2.ssidStream = FFS_STRING_INPUT_STREAM(SSID2);
    configuration2.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    bool hasNetwork = false;
    ASSERT_EQ(ffsWifiScanListHasNetwork(&wifiContext, &configuration1, &hasNetwork), FFS_SUCCESS);
    ASSERT_EQ(hasNetwork, true);

    hasNetwork = true;
    ASSERT_EQ(ffsWifiScanListHasNetwork(&wifiContext, &configuration2, &hasNetwork), FFS_SUCCESS);
    ASSERT_EQ(hasNetwork, false);

    return;
}

TEST(WifiScanListTests, Clear)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    FfsWifiScanResult_t scanResultList[2];

    scanResultList[0].ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    scanResultList[0].bssidStream = FFS_STRING_INPUT_STREAM(BSSID1);
    scanResultList[0].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    ASSERT_EQ(ffsWifiScanListPush(&wifiContext, &scanResultList[0]), FFS_SUCCESS);

    scanResultList[1].ssidStream = FFS_STRING_INPUT_STREAM(SSID2);
    scanResultList[1].bssidStream = FFS_STRING_INPUT_STREAM(BSSID2);
    scanResultList[1].securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WEP;

    ASSERT_EQ(ffsWifiScanListPush(&wifiContext, &scanResultList[1]), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiScanListClear(&wifiContext), FFS_SUCCESS);

    bool isEmpty;
    ASSERT_EQ(ffsWifiScanListIsEmpty(&wifiContext, &isEmpty), FFS_SUCCESS);
    ASSERT_TRUE(isEmpty);

    return;
}

TEST(WifiScanListTests, IsEmpty)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    bool isEmpty;
    FfsWifiScanResult_t scanResult;

    ASSERT_EQ(ffsWifiScanListIsEmpty(&wifiContext, &isEmpty), FFS_SUCCESS);
    ASSERT_TRUE(isEmpty);

    scanResult.ssidStream = FFS_STRING_INPUT_STREAM(SSID1);
    scanResult.bssidStream = FFS_STRING_INPUT_STREAM(BSSID1);
    scanResult.securityProtocol = FFS_WIFI_SECURITY_PROTOCOL_WPA_PSK;

    ASSERT_EQ(ffsWifiScanListPush(&wifiContext, &scanResult), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiScanListIsEmpty(&wifiContext, &isEmpty), FFS_SUCCESS);
    ASSERT_FALSE(isEmpty);

    ASSERT_EQ(ffsWifiScanListPop(&wifiContext), FFS_SUCCESS);

    ASSERT_EQ(ffsWifiScanListIsEmpty(&wifiContext, &isEmpty), FFS_SUCCESS);
    ASSERT_TRUE(isEmpty);

    return;
}

static bool areWifiScanResultsEqual(FfsWifiScanResult_t *scanResult1,
    FfsWifiScanResult_t *scanResult2)
{
    bool equal = true;

    ffsRewindStream(&scanResult1->ssidStream);
    ffsRewindStream(&scanResult2->ssidStream);
    ffsRewindStream(&scanResult1->bssidStream);
    ffsRewindStream(&scanResult2->bssidStream);

    equal &= ffsStreamMatchesStream(&scanResult1->ssidStream, &scanResult2->ssidStream);
    equal &= ffsStreamMatchesStream(&scanResult1->bssidStream, &scanResult2->bssidStream);
    equal &= (scanResult1->securityProtocol == scanResult2->securityProtocol);

    return equal;
}

