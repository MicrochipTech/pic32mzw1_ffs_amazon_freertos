/** @file ffs_raspbian_iwlist_tests.cpp
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

#include <gmock/gmock.h>
#include "ffs/linux/ffs_shell.h"
#include "ffs/linux/ffs_wifi_context.h"
#include "ffs/linux/ffs_wifi_scan_list.h"
#include "ffs/raspbian/ffs_raspbian_iwlist.h"

/** @brief Structure to be provded to the directed scan callback.
 */
typedef struct {
    FfsStream_t *ssidStream;
    bool *isFound;
} FfsDirectedScanCommandData_t;

const char *singleResult = "\
echo \'\
wlan0    Scan completed:\n\
         Cell 1 - Address: 11:22:33:44:55:66\n\
            Channel:-123\n\
            Quality=12/34 Signal level=-56 dBm\n\
            Encryption key:off\n\
            ESSID:\"TEST OPEN\"\n\
            IE: 134892348181349039238490381234\n\
            IE: 1348923481813490392313412423418490381234\n\
    \'";

const char *multipleResults = "\
echo \'\
wlan0    Scan completed:\n\
         Cell 2 - Address: 77:88:99:00:AA:BB\n\
            Channel:-123\n\
            Quality=12/34 Signal level=-56 dBm\n\
            Encryption key:on\n\
            ESSID:\"TEST WPA2\"\n\
            IE: 134892348181349039238490381234\n\
            IE: IEEE 802.11i/WPA2 Version 1\n\
            IE: 1348923481813490392313412423418490381234\n\
         Cell 3 - Address: AA:BB:CC:DD:EE:FF\n\
            Channel:-123\n\
            Quality=12/34 Signal level=-56 dBm\n\
            Encryption key:on\n\
            ESSID:\"TEST WEP\"\n\
            IE: 134892348181349039238490381234\n\
            IE: 1348923481813490392313412423418490381234\n\
    \'";

const char *directedScanResult = "\
echo \'\
wlan0    Scan completed:\n\
         Cell 1 - Address: 11:22:33:44:55:66\n\
            Channel:-123\n\
            Quality=12/34 Signal level=-56 dBm\n\
            Encryption key:off\n\
            ESSID:\"TEST WEP\"\n\
            IE: 134892348181349039238490381234\n\
            IE: 1348923481813490392313412423418490381234\n\
         Cell 2 - Address: 11:22:33:44:55:66\n\
            Channel:-123\n\
            Quality=12/34 Signal level=-56 dBm\n\
            Encryption key:off\n\
            ESSID:\"TEST OPEN\"\n\
            IE: 134892348181349039238490381234\n\
            IE: 1348923481813490392313412423418490381234\n\
    \'";

const char *directedScanNoResults = "\
echo \'\
wlan0    No scan results\n\
    \'";

TEST(IwlistTests, TestBackgroundWifiScanSingleResult)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    ASSERT_EQ(ffsExecuteShellCommand(singleResult, (FfsShellCallback_t)ffsRaspbianProcessBackgroundScanOutput, &wifiContext), FFS_SUCCESS);

    size_t size = 0;
    ASSERT_EQ(ffsWifiScanListGetSize(&wifiContext, &size), FFS_SUCCESS);
    ASSERT_EQ(size, 1);
}

TEST(IwlistTests, TestBackgroundWifiScanMultipleResults)
{
    FfsLinuxWifiContext_t wifiContext;
    ASSERT_EQ(ffsInitializeWifiContext(&wifiContext), FFS_SUCCESS);

    ASSERT_EQ(ffsExecuteShellCommand(singleResult, (FfsShellCallback_t)ffsRaspbianProcessBackgroundScanOutput, &wifiContext), FFS_SUCCESS);
    ASSERT_EQ(ffsExecuteShellCommand(multipleResults, (FfsShellCallback_t)ffsRaspbianProcessBackgroundScanOutput, &wifiContext), FFS_SUCCESS);

    size_t size = 0;
    ASSERT_EQ(ffsWifiScanListGetSize(&wifiContext, &size), FFS_SUCCESS);
    ASSERT_EQ(size, 3);
}

TEST(IwlistTests, TestDirectedScanFound)
{
    FfsStream_t ssidStream = FFS_STRING_INPUT_STREAM("TEST OPEN");
    bool isFound = false;

    FfsDirectedScanCommandData_t commandData = {
        .ssidStream = &ssidStream,
        .isFound = &isFound
    };

    ASSERT_EQ(ffsExecuteShellCommand(directedScanResult, (FfsShellCallback_t)ffsRaspbianProcessDirectedScanOutput, &commandData), FFS_SUCCESS);
    ASSERT_EQ(isFound, true);
}

TEST(IwlistTests, TestDirectedScanNotFound)
{
    FfsStream_t ssidStream = FFS_STRING_INPUT_STREAM("TEST WPA");
    bool isFound = true;

    FfsDirectedScanCommandData_t commandData = {
        .ssidStream = &ssidStream,
        .isFound = &isFound
    };

    ASSERT_EQ(ffsExecuteShellCommand(directedScanResult, (FfsShellCallback_t)ffsRaspbianProcessDirectedScanOutput, &commandData), FFS_SUCCESS);
    ASSERT_EQ(isFound, false);
}

TEST(IwlistTests, TestDirectedScanNoResults)
{
    FfsStream_t ssidStream = FFS_STRING_INPUT_STREAM("TEST OPEN");
    bool isFound = true;

    FfsDirectedScanCommandData_t commandData = {
        .ssidStream = &ssidStream,
        .isFound = &isFound
    };

    ASSERT_EQ(ffsExecuteShellCommand(directedScanNoResults, (FfsShellCallback_t)ffsRaspbianProcessDirectedScanOutput, &commandData), FFS_SUCCESS);
    ASSERT_EQ(isFound, false);
}
