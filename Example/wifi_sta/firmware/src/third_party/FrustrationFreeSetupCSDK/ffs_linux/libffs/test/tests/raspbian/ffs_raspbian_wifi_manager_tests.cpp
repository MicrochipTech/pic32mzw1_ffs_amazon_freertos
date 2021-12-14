/** @file ffs_wifi_manager_tests.cpp
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
#include "ffs/linux/ffs_wifi_context.h"
#include "ffs/raspbian/ffs_raspbian_wifi_manager.h"

#include <gmock/gmock.h>
#include <unistd.h>

#define SECONDS_TO_MICROSECONDS(s) ( s * 1000000 )

#define ZERO_FILL(variable) memset(&variable, 0, sizeof(variable))

static void ffsTestDeinitializeCallback(struct FfsUserContext_s *userContext, FFS_RESULT result);

static bool wifiManagerDeinitialized = false;

TEST(WifiManagerTests, InitAndDeinit)
{
    struct FfsUserContext_s userContext;
    ZERO_FILL(userContext);

    ASSERT_EQ(ffsInitializeWifiContext(&userContext.wifiContext), FFS_SUCCESS);

    userContext.wifiContext.wifiManagerInitialized = false;

    ASSERT_EQ(ffsRaspbianInitializeWifiManager(&userContext), FFS_SUCCESS);
    ASSERT_EQ(ffsRaspbianDeinitializeWifiManager(&userContext, ffsTestDeinitializeCallback), FFS_SUCCESS);

    for (uint32_t timer = 0; timer < SECONDS_TO_MICROSECONDS(2) && !wifiManagerDeinitialized; timer += SECONDS_TO_MICROSECONDS(0.2)) {
        usleep(SECONDS_TO_MICROSECONDS(0.2));
    }

    ASSERT_EQ(wifiManagerDeinitialized, true);
}

TEST(WifiManagerTests, MultipleInit)
{
    struct FfsUserContext_s userContext;
    ZERO_FILL(userContext);

    ASSERT_EQ(ffsInitializeWifiContext(&userContext.wifiContext), FFS_SUCCESS);

    userContext.wifiContext.wifiManagerInitialized = false;

    ASSERT_EQ(ffsRaspbianInitializeWifiManager(&userContext), FFS_SUCCESS);
    ASSERT_EQ(ffsRaspbianInitializeWifiManager(&userContext), FFS_SUCCESS);
    ASSERT_EQ(ffsRaspbianInitializeWifiManager(&userContext), FFS_SUCCESS);
}

TEST(WifiManagerTests, MultipleDeinit)
{
    struct FfsUserContext_s userContext;
    ZERO_FILL(userContext);

    ASSERT_EQ(ffsInitializeWifiContext(&userContext.wifiContext), FFS_SUCCESS);

    userContext.wifiContext.wifiManagerInitialized = false;

    ASSERT_EQ(ffsRaspbianInitializeWifiManager(&userContext), FFS_SUCCESS);
    ASSERT_EQ(ffsRaspbianDeinitializeWifiManager(&userContext, ffsTestDeinitializeCallback), FFS_SUCCESS);
    ASSERT_EQ(ffsRaspbianDeinitializeWifiManager(&userContext, ffsTestDeinitializeCallback), FFS_SUCCESS);
    ASSERT_EQ(ffsRaspbianDeinitializeWifiManager(&userContext, ffsTestDeinitializeCallback), FFS_SUCCESS);

    for (uint32_t timer = 0; timer < SECONDS_TO_MICROSECONDS(2) && !wifiManagerDeinitialized; timer += SECONDS_TO_MICROSECONDS(0.2)) {
        usleep(SECONDS_TO_MICROSECONDS(0.2));
    }

    ASSERT_EQ(wifiManagerDeinitialized, true);
}

static void ffsTestDeinitializeCallback(struct FfsUserContext_s *userContext, FFS_RESULT result) {
    (void) userContext;
    (void) result;

    wifiManagerDeinitialized = true;
}
